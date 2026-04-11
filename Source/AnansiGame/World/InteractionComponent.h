// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class AAnansiInteractable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableFound, AAnansiInteractable*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableLost);

/**
 * UAnansiInteractionComponent
 *
 * Attached to the player character. Performs sphere overlap traces each tick
 * to find the nearest interactable in range, and exposes Interact() for
 * the input system to call.
 */
UCLASS(ClassGroup = (Anansi), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UAnansiInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnansiInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Try to interact with the current focused interactable. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Interaction")
	void Interact();

	/** The interactable currently in focus (closest + in view). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Interaction")
	AAnansiInteractable* GetFocusedInteractable() const { return FocusedInteractable.Get(); }

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Interaction")
	FOnInteractableFound OnInteractableFound;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Interaction")
	FOnInteractableLost OnInteractableLost;

	/** Detection radius for interactables. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Interaction", meta = (ClampMin = "50.0"))
	float InteractionRadius = 250.0f;

	/** Maximum dot product angle for the interactable to be "in view". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Interaction", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float ViewDotThreshold = 0.5f;

private:
	TWeakObjectPtr<AAnansiInteractable> FocusedInteractable;

	void FindBestInteractable();
};
