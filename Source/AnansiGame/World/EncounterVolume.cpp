// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/EncounterVolume.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"

AEncounterVolume::AEncounterVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetHiddenInGame(true);
	RootComponent = TriggerBox;
}

void AEncounterVolume::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bAutoTrigger || bIsActive)
	{
		return;
	}

	if (bOneShot && bHasTriggered)
	{
		return;
	}

	if (Cast<AAnansiCharacter>(OtherActor))
	{
		StartEncounter();
	}
}

void AEncounterVolume::StartEncounter()
{
	if (bIsActive || Waves.Num() == 0)
	{
		return;
	}

	if (bOneShot && bHasTriggered)
	{
		return;
	}

	bIsActive = true;
	bHasTriggered = true;
	CurrentWaveIndex = -1;

	// Register overlap if not already done (for manual trigger case)
	TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &AEncounterVolume::OnTriggerOverlap);

	UE_LOG(LogAnansi, Log, TEXT("Encounter started: %s (%d waves)"), *GetName(), Waves.Num());
	OnEncounterStarted.Broadcast();

	AdvanceToNextWave();
}

void AEncounterVolume::AdvanceToNextWave()
{
	CurrentWaveIndex++;

	if (CurrentWaveIndex >= Waves.Num())
	{
		// All waves complete
		bIsActive = false;
		UE_LOG(LogAnansi, Log, TEXT("Encounter completed: %s"), *GetName());
		OnEncounterCompleted.Broadcast();
		return;
	}

	const FEncounterWave& Wave = Waves[CurrentWaveIndex];

	if (Wave.SpawnDelay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			WaveDelayTimerHandle,
			FTimerDelegate::CreateUObject(this, &AEncounterVolume::SpawnWave, CurrentWaveIndex),
			Wave.SpawnDelay,
			false);
	}
	else
	{
		SpawnWave(CurrentWaveIndex);
	}
}

void AEncounterVolume::SpawnWave(int32 WaveIndex)
{
	if (!Waves.IsValidIndex(WaveIndex))
	{
		return;
	}

	const FEncounterWave& Wave = Waves[WaveIndex];
	AliveEnemies.Reset();

	for (const FEnemySpawnSlot& Slot : Wave.Enemies)
	{
		if (!Slot.EnemyClass)
		{
			continue;
		}

		const FVector SpawnLocation = GetActorLocation() + Slot.SpawnOffset;
		const FRotator SpawnRotation = GetActorRotation() + Slot.SpawnRotation;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Enemy = GetWorld()->SpawnActor<APawn>(Slot.EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (Enemy)
		{
			Enemy->Tags.AddUnique(FName("Enemy"));
			Enemy->OnDestroyed.AddDynamic(this, &AEncounterVolume::OnEnemyDestroyed);
			AliveEnemies.Add(Enemy);
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("Wave %d spawned: %d enemies"), WaveIndex, AliveEnemies.Num());
	OnEncounterWaveStarted.Broadcast(WaveIndex);
}

void AEncounterVolume::OnEnemyDestroyed(AActor* DestroyedActor)
{
	AliveEnemies.RemoveAll([DestroyedActor](const TWeakObjectPtr<APawn>& Ptr)
	{
		return !Ptr.IsValid() || Ptr.Get() == DestroyedActor;
	});

	CheckWaveCompletion();
}

void AEncounterVolume::CheckWaveCompletion()
{
	// Clean up stale references
	AliveEnemies.RemoveAll([](const TWeakObjectPtr<APawn>& Ptr) { return !Ptr.IsValid(); });

	if (AliveEnemies.Num() == 0 && bIsActive)
	{
		UE_LOG(LogAnansi, Log, TEXT("Wave %d cleared"), CurrentWaveIndex);
		AdvanceToNextWave();
	}
}
