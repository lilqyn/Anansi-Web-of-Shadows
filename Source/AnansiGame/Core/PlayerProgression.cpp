// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/PlayerProgression.h"
#include "AnansiGame.h"
#include "UI/AnansiDevHUD.h"
#include "Kismet/GameplayStatics.h"

void UPlayerProgression::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPlayerProgression::AwardXP(int32 Amount)
{
	if (Amount <= 0) return;

	CurrentXP += Amount;
	OnXPGained.Broadcast(Amount, CurrentXP);

	UE_LOG(LogAnansi, Verbose, TEXT("XP: +%d (total: %d)"), Amount, CurrentXP);

	CheckLevelUp();
}

int32 UPlayerProgression::GetXPForNextLevel() const
{
	// Simple quadratic formula: level 1→2 = 100 XP, 2→3 = 400, 3→4 = 900, etc.
	return CurrentLevel * CurrentLevel * 100;
}

float UPlayerProgression::GetLevelProgress() const
{
	const int32 Required = GetXPForNextLevel();
	return Required > 0 ? static_cast<float>(CurrentXP) / Required : 0.0f;
}

bool UPlayerProgression::SpendUpgradePoint(int32 Count)
{
	if (UpgradePoints < Count) return false;
	UpgradePoints -= Count;
	return true;
}

void UPlayerProgression::CheckLevelUp()
{
	while (CurrentXP >= GetXPForNextLevel())
	{
		CurrentXP -= GetXPForNextLevel();
		CurrentLevel++;
		UpgradePoints += 2;
		OnLevelUp.Broadcast(CurrentLevel);

		// Toast + flash
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
			{
				if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
				{
					HUD->ShowToast(FString::Printf(TEXT("LEVEL UP! Level %d (+2 skill points)"), CurrentLevel),
						FColor(255, 220, 50));
					HUD->FlashScreen(FLinearColor(1.0f, 0.9f, 0.3f), 0.3f);
				}
			}
		}

		UE_LOG(LogAnansi, Log, TEXT("LEVEL UP! Now level %d (Upgrade Points: %d)"), CurrentLevel, UpgradePoints);
	}
}
