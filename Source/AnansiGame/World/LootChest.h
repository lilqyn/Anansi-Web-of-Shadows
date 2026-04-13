// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/AnansiInteractable.h"
#include "LootChest.generated.h"

class UStaticMeshComponent;

/**
 * ALootChest
 *
 * Interactable chest that spawns multiple loot drops when opened.
 * Can be locked (requires a key).
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ALootChest : public AAnansiInteractable
{
	GENERATED_BODY()

public:
	ALootChest();

	virtual void OnInteract(ACharacter* InteractingCharacter) override;
	virtual bool CanInteract() const override;

	/** Number of loot orbs to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Chest", meta = (ClampMin = "1"))
	int32 LootCount = 5;

	/** If true, chest is locked and requires a key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Chest")
	bool bLocked = false;

	/** Tag name of the key required to unlock. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Chest")
	FName RequiredKeyID = FName("BrassKey");

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ChestBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ChestLid;

private:
	bool bIsOpen = false;
};
