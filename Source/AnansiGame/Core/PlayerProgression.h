// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerProgression.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnXPGained, int32, Amount, int32, NewTotal);

/**
 * UPlayerProgression
 *
 * Tracks player XP, level, and upgrade points.
 * Awards XP for kills, combo chains, exploration.
 * Level-ups grant upgrade points for skill tree.
 */
UCLASS()
class ANANSIGAME_API UPlayerProgression : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Anansi|Progression")
	void AwardXP(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Anansi|Progression")
	int32 GetCurrentLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Progression")
	int32 GetCurrentXP() const { return CurrentXP; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Progression")
	int32 GetXPForNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Anansi|Progression")
	float GetLevelProgress() const;

	UFUNCTION(BlueprintPure, Category = "Anansi|Progression")
	int32 GetUpgradePoints() const { return UpgradePoints; }

	UFUNCTION(BlueprintCallable, Category = "Anansi|Progression")
	bool SpendUpgradePoint(int32 Count = 1);

	UPROPERTY(BlueprintAssignable) FOnLevelUp OnLevelUp;
	UPROPERTY(BlueprintAssignable) FOnXPGained OnXPGained;

private:
	int32 CurrentLevel = 1;
	int32 CurrentXP = 0;
	int32 UpgradePoints = 0;

	void CheckLevelUp();
};
