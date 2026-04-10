// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "DialogueManager.generated.h"

/**
 * A single choice the player can make during dialogue.
 */
USTRUCT(BlueprintType)
struct FDialogueChoice
{
	GENERATED_BODY()

	/** Display text for the choice. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText ChoiceText;

	/** ID of the dialogue node this choice leads to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName NextNodeID;

	/** Gameplay tag condition — choice only appears if this tag is active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName RequiredCondition;

	/** If true, this choice is only available when Borrowed Voice is active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRequiresBorrowedVoice = false;

	/** Choice consequence key (fed into the ChoiceManager). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName ConsequenceKey;
};

/**
 * A single node in a dialogue tree.
 */
USTRUCT(BlueprintType)
struct FDialogueNode : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique node identifier. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName NodeID;

	/** Name of the speaking character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText SpeakerName;

	/** The dialogue text itself. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DialogueText;

	/** Available player choices. Empty = auto-advance to NextNodeID. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FDialogueChoice> Choices;

	/** If no choices, automatically advance to this node. NAME_None = end of conversation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName NextNodeID;

	/** Sound / voice line to play. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USoundBase> VoiceLine;

	/** Camera shot tag for cinematic framing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName CameraShotTag;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueNodeStarted, const FDialogueNode&, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueChoiceMade, FName, NodeID, int32, ChoiceIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

/**
 * UDialogueManager
 *
 * Game-instance subsystem that drives dialogue trees loaded from data tables,
 * tracks conversation state, and supports Borrowed Voice special options.
 */
UCLASS()
class ANANSIGAME_API UDialogueManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// -------------------------------------------------------------------
	// Dialogue flow
	// -------------------------------------------------------------------

	/** Start a conversation using the given data table. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Dialogue")
	void StartDialogue(UDataTable* DialogueTable, FName StartNodeID);

	/** Advance to a specific node. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Dialogue")
	void AdvanceToNode(FName NodeID);

	/** Player selects a choice by index. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Dialogue")
	void SelectChoice(int32 ChoiceIndex);

	/** End the current dialogue. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Dialogue")
	void EndDialogue();

	/** Is a dialogue currently in progress? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Dialogue")
	bool IsInDialogue() const { return bIsInDialogue; }

	/** Get the current dialogue node. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Dialogue")
	const FDialogueNode& GetCurrentNode() const { return CurrentNode; }

	// -------------------------------------------------------------------
	// Conversation state
	// -------------------------------------------------------------------

	/** Check if a conversation has been completed. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Dialogue")
	bool HasCompletedConversation(FName ConversationID) const;

	/** Mark a conversation as completed. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Dialogue")
	void MarkConversationCompleted(FName ConversationID);

	// -------------------------------------------------------------------
	// Borrowed Voice
	// -------------------------------------------------------------------

	/** Set whether Borrowed Voice ability is currently active. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Dialogue")
	void SetBorrowedVoiceActive(bool bActive);

	UFUNCTION(BlueprintPure, Category = "Anansi|Dialogue")
	bool IsBorrowedVoiceActive() const { return bBorrowedVoiceActive; }

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Dialogue")
	FOnDialogueNodeStarted OnDialogueNodeStarted;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Dialogue")
	FOnDialogueChoiceMade OnDialogueChoiceMade;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Dialogue")
	FOnDialogueEnded OnDialogueEnded;

private:
	/** Look up a node from the active data table. */
	bool FindNode(FName NodeID, FDialogueNode& OutNode) const;

	/** Filter choices based on conditions and Borrowed Voice state. */
	TArray<FDialogueChoice> GetAvailableChoices(const FDialogueNode& Node) const;

	UPROPERTY()
	TObjectPtr<UDataTable> ActiveDialogueTable;

	FDialogueNode CurrentNode;
	bool bIsInDialogue = false;
	bool bBorrowedVoiceActive = false;

	TSet<FName> CompletedConversations;
};
