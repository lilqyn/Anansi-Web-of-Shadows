// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/WaveSurvival.h"
#include "AnansiGame.h"
#include "AI/EnemyGuard.h"
#include "AI/EnemyRanged.h"
#include "AI/EnemyTurret.h"
#include "UI/AnansiDevHUD.h"
#include "Kismet/GameplayStatics.h"

AWaveSurvival::AWaveSurvival()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AWaveSurvival::StartSurvival()
{
	if (bIsActive) return;

	bIsActive = true;
	CurrentWave = 0;
	SpawnWave();

	// Periodic check for wave completion
	GetWorldTimerManager().SetTimer(WaveCheckTimer, this, &AWaveSurvival::CheckWaveComplete, 1.0f, true);

	UE_LOG(LogAnansi, Log, TEXT("Wave Survival started!"));
}

void AWaveSurvival::SpawnWave()
{
	CurrentWave++;

	// Escalating difficulty
	const int32 EnemyCount = BaseEnemiesPerWave + (CurrentWave - 1) * 2;
	const float SpawnRadius = 800.0f;
	const FVector Center = GetActorLocation();

	SpawnedEnemies.Empty();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < EnemyCount; ++i)
	{
		const float Angle = (static_cast<float>(i) / EnemyCount) * 360.0f;
		const float Rad = FMath::DegreesToRadians(Angle);
		const FVector Loc = Center + FVector(FMath::Cos(Rad) * SpawnRadius, FMath::Sin(Rad) * SpawnRadius, 0);
		const FRotator Rot = (Center - Loc).Rotation();

		AActor* Enemy = nullptr;

		// Mix enemy types based on wave number
		if (CurrentWave >= 5 && i % 4 == 0)
		{
			// Every 4th enemy is a turret from wave 5+
			Enemy = GetWorld()->SpawnActor<AEnemyTurret>(AEnemyTurret::StaticClass(), Loc + FVector(0, 0, 100), Rot, Params);
		}
		else if (CurrentWave >= 3 && i % 3 == 0)
		{
			// Every 3rd enemy is ranged from wave 3+
			Enemy = GetWorld()->SpawnActor<AEnemyRanged>(AEnemyRanged::StaticClass(), Loc, Rot, Params);
		}
		else
		{
			Enemy = GetWorld()->SpawnActor<AEnemyGuard>(AEnemyGuard::StaticClass(), Loc, Rot, Params);
		}

		if (Enemy)
		{
			SpawnedEnemies.Add(Enemy);
		}
	}

	// Toast notification
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->ShowToast(FString::Printf(TEXT("WAVE %d — %d enemies!"), CurrentWave, EnemyCount),
				FColor(255, 100, 100));
		}
	}

	OnWaveStarted.Broadcast(CurrentWave, EnemyCount);
	UE_LOG(LogAnansi, Log, TEXT("Wave %d: %d enemies spawned"), CurrentWave, EnemyCount);
}

void AWaveSurvival::CheckWaveComplete()
{
	if (!bIsActive) return;

	if (GetAliveCount() == 0)
	{
		// Wave clear toast
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
			{
				HUD->ShowToast(FString::Printf(TEXT("Wave %d cleared!"), CurrentWave), FColor(100, 255, 100));
			}
		}

		// Delay before next wave
		GetWorldTimerManager().SetTimer(WaveDelayTimer, this, &AWaveSurvival::SpawnWave, TimeBetweenWaves, false);
	}
}

int32 AWaveSurvival::GetAliveCount() const
{
	int32 Count = 0;
	for (const auto& Weak : SpawnedEnemies)
	{
		if (Weak.IsValid())
		{
			Count++;
		}
	}
	return Count;
}
