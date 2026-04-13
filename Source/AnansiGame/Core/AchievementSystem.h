// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AchievementSystem.generated.h"

USTRUCT(BlueprintType)
struct FAchievement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName ID;
	UPROPERTY(BlueprintReadOnly) FString Title;
	UPROPERTY(BlueprintReadOnly) FString Description;
	UPROPERTY(BlueprintReadOnly) bool bUnlocked = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlocked, const FAchievement&, Achievement);

/**
 * UAchievementSystem
 *
 * Tracks achievements and unlocks them based on gameplay events.
 * Shows toast notifications on unlock.
 */
UCLASS()
class ANANSIGAME_API UAchievementSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Anansi|Achievements")
	bool UnlockAchievement(FName ID);

	UFUNCTION(BlueprintPure, Category = "Anansi|Achievements")
	bool IsUnlocked(FName ID) const;

	UFUNCTION(BlueprintPure, Category = "Anansi|Achievements")
	int32 GetUnlockedCount() const;

	UFUNCTION(BlueprintPure, Category = "Anansi|Achievements")
	int32 GetTotalCount() const { return Achievements.Num(); }

	UFUNCTION(BlueprintPure, Category = "Anansi|Achievements")
	TArray<FAchievement> GetAllAchievements() const;

	/** Called by gameplay events to check for achievements. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Achievements")
	void CheckKillCount(int32 TotalKills);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Achievements")
	void CheckCombo(int32 MaxCombo);

	UPROPERTY(BlueprintAssignable) FOnAchievementUnlocked OnAchievementUnlocked;

private:
	UPROPERTY()
	TMap<FName, FAchievement> Achievements;

	void RegisterAchievements();
	void ShowUnlockToast(const FAchievement& Achievement);
};
