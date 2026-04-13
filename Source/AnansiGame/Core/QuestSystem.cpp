// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/QuestSystem.h"
#include "AnansiGame.h"
#include "Core/CombatStats.h"

void UQuestSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UQuestSystem::SetObjective(const FQuestObjective& NewObjective)
{
	CurrentObjective = NewObjective;
	CurrentObjective.CurrentCount = 0;
	CurrentObjective.bComplete = false;
	bHasActive = true;

	OnProgress.Broadcast(CurrentObjective);
	UE_LOG(LogAnansi, Log, TEXT("Quest: %s (0/%d)"),
		*CurrentObjective.DisplayText.ToString(), CurrentObjective.TargetCount);
}

void UQuestSystem::ProgressObjective(int32 Amount)
{
	if (!bHasActive || CurrentObjective.bComplete) return;

	CurrentObjective.CurrentCount = FMath::Min(CurrentObjective.CurrentCount + Amount,
		CurrentObjective.TargetCount);
	OnProgress.Broadcast(CurrentObjective);

	UE_LOG(LogAnansi, Log, TEXT("Quest progress: %s (%d/%d)"),
		*CurrentObjective.DisplayText.ToString(),
		CurrentObjective.CurrentCount, CurrentObjective.TargetCount);

	if (CurrentObjective.CurrentCount >= CurrentObjective.TargetCount)
	{
		CompleteObjective();
	}
}

void UQuestSystem::CompleteObjective()
{
	if (!bHasActive) return;

	CurrentObjective.bComplete = true;
	CurrentObjective.CurrentCount = CurrentObjective.TargetCount;
	OnComplete.Broadcast();

	UE_LOG(LogAnansi, Log, TEXT("Quest COMPLETE: %s"), *CurrentObjective.DisplayText.ToString());
}

void UQuestSystem::ClearObjective()
{
	bHasActive = false;
	CurrentObjective = FQuestObjective();
}
