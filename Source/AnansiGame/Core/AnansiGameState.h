// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AnansiGameState.generated.h"

/**
 * Represents a story fragment that Anansi can collect to learn lore and
 * unlock narrative branches.
 */
USTRUCT(BlueprintType)
struct FAnansiStoryFragment
{
	GENERATED_BODY()

	/** Unique identifier for this fragment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FName FragmentID;

	/** Localised display title. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FText Title;

	/** Localised body text / journal entry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FText Description;

	/** True once the player has collected this fragment. */
	UPROPERTY(BlueprintReadWrite, Category = "Story")
	bool bCollected = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryFragmentCollected, const FAnansiStoryFragment&, Fragment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorldFlagChanged, FName, FlagName, bool, bNewValue);

/**
 * AAnansiGameState
 *
 * Authority-owned state that tracks the current story progress, collected
 * story fragments, and world-state flags (region unlocks, NPC status,
 * branching-choice results). Replicated in networked builds.
 */
UCLASS()
class ANANSIGAME_API AAnansiGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AAnansiGameState();

	// -----------------------------------------------------------------------
	// Story progress
	// -----------------------------------------------------------------------

	/** Index into the main quest line (0 = prologue). */
	UPROPERTY(BlueprintReadWrite, Category = "Anansi|Story")
	int32 MainQuestIndex = 0;

	/** Current chapter name for HUD display. */
	UPROPERTY(BlueprintReadWrite, Category = "Anansi|Story")
	FText CurrentChapterName;

	// -----------------------------------------------------------------------
	// Story fragments
	// -----------------------------------------------------------------------

	/** All story fragments defined for this level / region. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Story")
	TArray<FAnansiStoryFragment> StoryFragments;

	/** Mark a fragment as collected by ID. Returns true if found and newly collected. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Story")
	bool CollectStoryFragment(FName FragmentID);

	/** Check whether a specific fragment has been collected. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Story")
	bool IsFragmentCollected(FName FragmentID) const;

	/** Get all collected fragment IDs. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Story")
	TArray<FName> GetCollectedFragmentIDs() const;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Story")
	FOnStoryFragmentCollected OnStoryFragmentCollected;

	// -----------------------------------------------------------------------
	// World state flags
	// -----------------------------------------------------------------------

	/** Set a named world flag (e.g., "Forest_Unlocked", "NPC_Kwaku_Alive"). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|World")
	void SetWorldFlag(FName FlagName, bool bValue);

	/** Query a world flag. Defaults to false if not set. */
	UFUNCTION(BlueprintPure, Category = "Anansi|World")
	bool GetWorldFlag(FName FlagName) const;

	/** Get all world flags as a map (for serialisation). */
	UFUNCTION(BlueprintPure, Category = "Anansi|World")
	const TMap<FName, bool>& GetAllWorldFlags() const { return WorldFlags; }

	/** Load world flags from a saved map (called during save-game restore). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|World")
	void SetAllWorldFlags(const TMap<FName, bool>& InFlags);

	UPROPERTY(BlueprintAssignable, Category = "Anansi|World")
	FOnWorldFlagChanged OnWorldFlagChanged;

private:
	/** Runtime world-state flag storage. */
	UPROPERTY()
	TMap<FName, bool> WorldFlags;
};
