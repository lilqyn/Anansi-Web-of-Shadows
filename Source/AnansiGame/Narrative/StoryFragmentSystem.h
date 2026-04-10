// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "StoryFragmentSystem.generated.h"

/**
 * Data for a single story fragment (tale piece).
 */
USTRUCT(BlueprintType)
struct FStoryFragment : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName FragmentID;

	/** Display name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText FragmentName;

	/** Short description for the collection UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;

	/** The region of the world where this fragment is found. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName Region;

	/** Narrative text — the tale content itself. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText NarrativeText;

	/** Gameplay effect granted when this fragment is collected (tag-based). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName GameplayEffectTag;

	/** Icon texture for the collection screen. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFragmentCollected, const FStoryFragment&, Fragment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFragmentMilestone, int32, TotalFragments);

/**
 * UStoryFragmentSystem
 *
 * Game-instance subsystem tracking collected story fragments. Provides
 * collection events, fragment queries for the Story Bind ability, and
 * save/load integration points.
 */
UCLASS()
class ANANSIGAME_API UStoryFragmentSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// -------------------------------------------------------------------
	// Collection
	// -------------------------------------------------------------------

	/** Collect a fragment by ID. Returns true if newly collected. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Story")
	bool CollectFragment(FName FragmentID);

	/** Has a specific fragment been collected? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Story")
	bool HasFragment(FName FragmentID) const;

	/** Total number of collected fragments. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Story")
	int32 GetCollectedFragmentCount() const { return CollectedFragmentIDs.Num(); }

	/** Get all collected fragment IDs. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Story")
	TArray<FName> GetCollectedFragmentIDs() const { return CollectedFragmentIDs.Array(); }

	/** Get the data for a specific fragment. Returns false if not found in the database. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Story")
	bool GetFragmentData(FName FragmentID, FStoryFragment& OutFragment) const;

	// -------------------------------------------------------------------
	// Region queries
	// -------------------------------------------------------------------

	/** Get the number of fragments collected in a specific region. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Story")
	int32 GetFragmentCountForRegion(FName Region) const;

	// -------------------------------------------------------------------
	// Story Bind integration
	// -------------------------------------------------------------------

	/** Check if the player has enough fragments to use Story Bind. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Story")
	bool MeetsStoryBindRequirement(int32 RequiredCount) const;

	// -------------------------------------------------------------------
	// Save / Load
	// -------------------------------------------------------------------

	/** Serialize collected fragments for save system. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Story")
	TArray<FName> SerializeForSave() const;

	/** Load collected fragments from save data. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Story")
	void LoadFromSave(const TArray<FName>& SavedFragmentIDs);

	// -------------------------------------------------------------------
	// Configuration
	// -------------------------------------------------------------------

	/** Data table containing all fragment definitions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Story")
	TObjectPtr<UDataTable> FragmentDatabase;

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Story")
	FOnFragmentCollected OnFragmentCollected;

	/** Fires at milestone counts (e.g. every 5 fragments). */
	UPROPERTY(BlueprintAssignable, Category = "Anansi|Story")
	FOnFragmentMilestone OnFragmentMilestone;

protected:
	/** Fragment count interval for milestone events. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Story")
	int32 MilestoneInterval = 5;

private:
	TSet<FName> CollectedFragmentIDs;
};
