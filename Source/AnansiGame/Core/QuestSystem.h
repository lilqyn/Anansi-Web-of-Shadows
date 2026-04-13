// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestSystem.generated.h"

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
	KillEnemies			UMETA(DisplayName = "Kill Enemies"),
	CollectFragments	UMETA(DisplayName = "Collect Story Fragments"),
	ReachLocation		UMETA(DisplayName = "Reach Location"),
	DefeatBoss			UMETA(DisplayName = "Defeat Boss"),
	SurviveWaves		UMETA(DisplayName = "Survive Waves"),
	SolvePuzzle			UMETA(DisplayName = "Solve Puzzle")
};

USTRUCT(BlueprintType)
struct FQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EObjectiveType Type = EObjectiveType::KillEnemies;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 TargetCount = 1;
	UPROPERTY(BlueprintReadOnly) int32 CurrentCount = 0;
	UPROPERTY(BlueprintReadOnly) bool bComplete = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveProgress, const FQuestObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestComplete);

/**
 * UQuestSystem
 *
 * Tracks the player's current quest objective.
 * Updates from gameplay events (kills, fragment pickups, etc.).
 */
UCLASS()
class ANANSIGAME_API UQuestSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Set the active objective. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Quest")
	void SetObjective(const FQuestObjective& NewObjective);

	/** Advance the current objective by 1 (or custom amount). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Quest")
	void ProgressObjective(int32 Amount = 1);

	/** Check if an objective is active. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Quest")
	bool HasActiveObjective() const { return bHasActive; }

	/** Get the current objective (only valid if HasActiveObjective). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Quest")
	const FQuestObjective& GetCurrentObjective() const { return CurrentObjective; }

	/** Mark current objective complete immediately. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Quest")
	void CompleteObjective();

	/** Clear the current objective. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Quest")
	void ClearObjective();

	UPROPERTY(BlueprintAssignable) FOnObjectiveProgress OnProgress;
	UPROPERTY(BlueprintAssignable) FOnQuestComplete OnComplete;

private:
	FQuestObjective CurrentObjective;
	bool bHasActive = false;
};
