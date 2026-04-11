// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AnansiInteractable.generated.h"

class ACharacter;
class UWidgetComponent;

/**
 * Interaction type — determines the prompt icon and behaviour.
 */
UENUM(BlueprintType)
enum class EInteractableType : uint8
{
	Generic		UMETA(DisplayName = "Generic"),
	Pickup		UMETA(DisplayName = "Pickup"),
	Door		UMETA(DisplayName = "Door"),
	Lever		UMETA(DisplayName = "Lever"),
	NPC			UMETA(DisplayName = "NPC Dialogue"),
	Puzzle		UMETA(DisplayName = "Puzzle Element"),
	StoryFragment UMETA(DisplayName = "Story Fragment")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteracted, ACharacter*, InteractingCharacter);

/**
 * AAnansiInteractable
 *
 * Base class for all interactable objects in the world. Provides focus/unfocus
 * callbacks, interaction prompts, and a type enum for HUD display.
 *
 * Subclass in Blueprint for specific behaviour (doors, pickups, puzzles).
 */
UCLASS(Abstract, Blueprintable)
class ANANSIGAME_API AAnansiInteractable : public AActor
{
	GENERATED_BODY()

public:
	AAnansiInteractable();

	/** Called by the interaction component when the player interacts. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Interaction")
	virtual void OnInteract(ACharacter* InteractingCharacter);

	/** Can this interactable currently be used? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Interaction")
	virtual bool CanInteract() const;

	/** Called when this becomes the player's focused interactable. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Interaction")
	virtual void OnGainedFocus();

	/** Called when this loses focus. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Interaction")
	virtual void OnLostFocus();

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Interaction")
	FOnInteracted OnInteracted;

	/** The text shown to the player when focused (e.g. "Open", "Pick Up", "Talk"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Interaction")
	FText InteractionPrompt = NSLOCTEXT("Anansi", "DefaultPrompt", "Interact");

	/** What kind of interactable this is — drives HUD icon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Interaction")
	EInteractableType InteractableType = EInteractableType::Generic;

	/** Whether this interactable can be used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Interaction")
	bool bIsEnabled = true;

	/** If true, disable after first interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Interaction")
	bool bSingleUse = false;

protected:
	/** Root scene component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Interaction")
	TObjectPtr<USceneComponent> SceneRoot;

	bool bHasBeenUsed = false;
};
