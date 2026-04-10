// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Narrative/StoryFragmentSystem.h"
#include "AnansiGame.h"

void UStoryFragmentSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogAnansi, Log, TEXT("StoryFragmentSystem initialized"));
}

// ---------------------------------------------------------------------------
// Collection
// ---------------------------------------------------------------------------

bool UStoryFragmentSystem::CollectFragment(FName FragmentID)
{
	if (FragmentID.IsNone())
	{
		return false;
	}

	if (CollectedFragmentIDs.Contains(FragmentID))
	{
		UE_LOG(LogAnansi, Verbose, TEXT("Fragment '%s' already collected"), *FragmentID.ToString());
		return false;
	}

	CollectedFragmentIDs.Add(FragmentID);

	// Look up data and broadcast
	FStoryFragment FragmentData;
	if (GetFragmentData(FragmentID, FragmentData))
	{
		OnFragmentCollected.Broadcast(FragmentData);
		UE_LOG(LogAnansi, Log, TEXT("Collected story fragment: %s (%s)"),
			*FragmentID.ToString(), *FragmentData.FragmentName.ToString());
	}
	else
	{
		UE_LOG(LogAnansi, Warning, TEXT("Fragment '%s' collected but not found in database"), *FragmentID.ToString());
	}

	// Milestone check
	const int32 Count = CollectedFragmentIDs.Num();
	if (MilestoneInterval > 0 && (Count % MilestoneInterval) == 0)
	{
		OnFragmentMilestone.Broadcast(Count);
		UE_LOG(LogAnansi, Log, TEXT("Story fragment milestone: %d collected"), Count);
	}

	return true;
}

bool UStoryFragmentSystem::HasFragment(FName FragmentID) const
{
	return CollectedFragmentIDs.Contains(FragmentID);
}

bool UStoryFragmentSystem::GetFragmentData(FName FragmentID, FStoryFragment& OutFragment) const
{
	if (!FragmentDatabase)
	{
		return false;
	}

	const FStoryFragment* Row = FragmentDatabase->FindRow<FStoryFragment>(FragmentID, TEXT("GetFragmentData"));
	if (!Row)
	{
		return false;
	}

	OutFragment = *Row;
	return true;
}

// ---------------------------------------------------------------------------
// Region queries
// ---------------------------------------------------------------------------

int32 UStoryFragmentSystem::GetFragmentCountForRegion(FName Region) const
{
	if (!FragmentDatabase)
	{
		return 0;
	}

	int32 Count = 0;
	for (const FName& ID : CollectedFragmentIDs)
	{
		FStoryFragment Fragment;
		if (GetFragmentData(ID, Fragment) && Fragment.Region == Region)
		{
			Count++;
		}
	}

	return Count;
}

// ---------------------------------------------------------------------------
// Story Bind
// ---------------------------------------------------------------------------

bool UStoryFragmentSystem::MeetsStoryBindRequirement(int32 RequiredCount) const
{
	return CollectedFragmentIDs.Num() >= RequiredCount;
}

// ---------------------------------------------------------------------------
// Save / Load
// ---------------------------------------------------------------------------

TArray<FName> UStoryFragmentSystem::SerializeForSave() const
{
	return CollectedFragmentIDs.Array();
}

void UStoryFragmentSystem::LoadFromSave(const TArray<FName>& SavedFragmentIDs)
{
	CollectedFragmentIDs.Empty();
	for (const FName& ID : SavedFragmentIDs)
	{
		CollectedFragmentIDs.Add(ID);
	}

	UE_LOG(LogAnansi, Log, TEXT("Loaded %d story fragments from save"), CollectedFragmentIDs.Num());
}
