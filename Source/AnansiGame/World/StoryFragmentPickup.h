// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/AnansiInteractable.h"
#include "StoryFragmentPickup.generated.h"

/**
 * AStoryFragmentPickup
 *
 * Collectible story fragment placed in the world. When the player
 * interacts with it, the fragment is added to the StoryFragmentSystem
 * and the pickup disappears with a visual effect.
 *
 * Each fragment has a title, lore text, and region tag.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AStoryFragmentPickup : public AAnansiInteractable
{
	GENERATED_BODY()

public:
	AStoryFragmentPickup();

	virtual void OnInteract(ACharacter* InteractingCharacter) override;

	/** Unique ID for this fragment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Fragment")
	FName FragmentID;

	/** Display title of the fragment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Fragment")
	FText FragmentTitle = NSLOCTEXT("Anansi", "FragDefault", "Story Fragment");

	/** Lore text revealed when collected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Fragment", meta = (MultiLine = true))
	FText LoreText;

	/** Region this fragment belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Fragment")
	FName RegionTag = FName("Nkran");

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Fragment")
	TObjectPtr<UStaticMeshComponent> FragmentMesh;

	/** Rotation speed for the floating effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Fragment")
	float RotationSpeed = 90.0f;

	/** Bobbing amplitude. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Fragment")
	float BobAmplitude = 15.0f;

	virtual void Tick(float DeltaTime) override;

private:
	float BobTimer = 0.0f;
	FVector InitialLocation;
};
