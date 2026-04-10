// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/AnansiGameState.h"
#include "AnansiGame.h"

AAnansiGameState::AAnansiGameState()
{
}

// ---------------------------------------------------------------------------
// Story fragments
// ---------------------------------------------------------------------------

bool AAnansiGameState::CollectStoryFragment(FName FragmentID)
{
	for (FAnansiStoryFragment& Frag : StoryFragments)
	{
		if (Frag.FragmentID == FragmentID)
		{
			if (Frag.bCollected)
			{
				// Already collected — no-op.
				return false;
			}

			Frag.bCollected = true;
			UE_LOG(LogAnansi, Log, TEXT("Story fragment collected: %s"), *FragmentID.ToString());
			OnStoryFragmentCollected.Broadcast(Frag);
			return true;
		}
	}

	UE_LOG(LogAnansi, Warning, TEXT("Story fragment '%s' not found in StoryFragments array."), *FragmentID.ToString());
	return false;
}

bool AAnansiGameState::IsFragmentCollected(FName FragmentID) const
{
	for (const FAnansiStoryFragment& Frag : StoryFragments)
	{
		if (Frag.FragmentID == FragmentID)
		{
			return Frag.bCollected;
		}
	}
	return false;
}

TArray<FName> AAnansiGameState::GetCollectedFragmentIDs() const
{
	TArray<FName> Result;
	for (const FAnansiStoryFragment& Frag : StoryFragments)
	{
		if (Frag.bCollected)
		{
			Result.Add(Frag.FragmentID);
		}
	}
	return Result;
}

// ---------------------------------------------------------------------------
// World flags
// ---------------------------------------------------------------------------

void AAnansiGameState::SetWorldFlag(FName FlagName, bool bValue)
{
	const bool* Existing = WorldFlags.Find(FlagName);
	const bool bChanged = !Existing || (*Existing != bValue);

	WorldFlags.Add(FlagName, bValue);

	if (bChanged)
	{
		UE_LOG(LogAnansi, Log, TEXT("World flag '%s' set to %s"), *FlagName.ToString(), bValue ? TEXT("true") : TEXT("false"));
		OnWorldFlagChanged.Broadcast(FlagName, bValue);
	}
}

bool AAnansiGameState::GetWorldFlag(FName FlagName) const
{
	const bool* Found = WorldFlags.Find(FlagName);
	return Found ? *Found : false;
}

void AAnansiGameState::SetAllWorldFlags(const TMap<FName, bool>& InFlags)
{
	WorldFlags = InFlags;
}
