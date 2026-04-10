// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Narrative/DialogueManager.h"
#include "AnansiGame.h"
#include "Kismet/GameplayStatics.h"

void UDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogAnansi, Log, TEXT("DialogueManager initialized"));
}

// ---------------------------------------------------------------------------
// Dialogue flow
// ---------------------------------------------------------------------------

void UDialogueManager::StartDialogue(UDataTable* DialogueTable, FName StartNodeID)
{
	if (!DialogueTable)
	{
		UE_LOG(LogAnansi, Warning, TEXT("StartDialogue — null data table"));
		return;
	}

	if (bIsInDialogue)
	{
		EndDialogue();
	}

	ActiveDialogueTable = DialogueTable;
	bIsInDialogue = true;

	AdvanceToNode(StartNodeID);

	UE_LOG(LogAnansi, Log, TEXT("Dialogue started (table: %s, start: %s)"),
		*DialogueTable->GetName(), *StartNodeID.ToString());
}

void UDialogueManager::AdvanceToNode(FName NodeID)
{
	if (!bIsInDialogue || !ActiveDialogueTable)
	{
		return;
	}

	if (NodeID.IsNone())
	{
		EndDialogue();
		return;
	}

	FDialogueNode Node;
	if (!FindNode(NodeID, Node))
	{
		UE_LOG(LogAnansi, Warning, TEXT("Dialogue node '%s' not found"), *NodeID.ToString());
		EndDialogue();
		return;
	}

	CurrentNode = Node;
	OnDialogueNodeStarted.Broadcast(CurrentNode);

	// Play voice line
	if (CurrentNode.VoiceLine)
	{
		UGameplayStatics::PlaySound2D(GetGameInstance(), CurrentNode.VoiceLine);
	}

	UE_LOG(LogAnansi, Verbose, TEXT("Dialogue -> node '%s' (%s)"),
		*NodeID.ToString(), *CurrentNode.SpeakerName.ToString());
}

void UDialogueManager::SelectChoice(int32 ChoiceIndex)
{
	if (!bIsInDialogue)
	{
		return;
	}

	const TArray<FDialogueChoice> AvailableChoices = GetAvailableChoices(CurrentNode);

	if (!AvailableChoices.IsValidIndex(ChoiceIndex))
	{
		UE_LOG(LogAnansi, Warning, TEXT("Invalid choice index %d (available: %d)"),
			ChoiceIndex, AvailableChoices.Num());
		return;
	}

	const FDialogueChoice& Choice = AvailableChoices[ChoiceIndex];

	OnDialogueChoiceMade.Broadcast(CurrentNode.NodeID, ChoiceIndex);

	// Advance to the next node
	AdvanceToNode(Choice.NextNodeID);
}

void UDialogueManager::EndDialogue()
{
	if (!bIsInDialogue)
	{
		return;
	}

	bIsInDialogue = false;
	ActiveDialogueTable = nullptr;
	CurrentNode = FDialogueNode();

	OnDialogueEnded.Broadcast();

	UE_LOG(LogAnansi, Log, TEXT("Dialogue ended"));
}

// ---------------------------------------------------------------------------
// Conversation state
// ---------------------------------------------------------------------------

bool UDialogueManager::HasCompletedConversation(FName ConversationID) const
{
	return CompletedConversations.Contains(ConversationID);
}

void UDialogueManager::MarkConversationCompleted(FName ConversationID)
{
	CompletedConversations.Add(ConversationID);
}

// ---------------------------------------------------------------------------
// Borrowed Voice
// ---------------------------------------------------------------------------

void UDialogueManager::SetBorrowedVoiceActive(bool bActive)
{
	bBorrowedVoiceActive = bActive;
}

// ---------------------------------------------------------------------------
// Internal
// ---------------------------------------------------------------------------

bool UDialogueManager::FindNode(FName NodeID, FDialogueNode& OutNode) const
{
	if (!ActiveDialogueTable)
	{
		return false;
	}

	const FDialogueNode* Row = ActiveDialogueTable->FindRow<FDialogueNode>(NodeID, TEXT("FindNode"));
	if (!Row)
	{
		return false;
	}

	OutNode = *Row;
	return true;
}

TArray<FDialogueChoice> UDialogueManager::GetAvailableChoices(const FDialogueNode& Node) const
{
	TArray<FDialogueChoice> Available;

	for (const FDialogueChoice& Choice : Node.Choices)
	{
		// Check Borrowed Voice requirement
		if (Choice.bRequiresBorrowedVoice && !bBorrowedVoiceActive)
		{
			continue;
		}

		// Check condition (simple name-based flag — a production system would use gameplay tags)
		if (!Choice.RequiredCondition.IsNone())
		{
			// Condition checking would integrate with a world-state / flag system
			// For now, all named conditions are assumed met
		}

		Available.Add(Choice);
	}

	return Available;
}
