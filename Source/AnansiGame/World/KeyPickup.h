// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeyPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * AKeyPickup
 *
 * Collectible key that adds to the player's inventory.
 * Used to unlock chests, doors, etc.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AKeyPickup : public AActor
{
	GENERATED_BODY()

public:
	AKeyPickup();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Key")
	FName KeyID = FName("BrassKey");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Key")
	FText KeyName = NSLOCTEXT("Anansi", "BrassKey", "Brass Key");

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollectionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> KeyMesh;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	FVector SpawnLoc;
	float BobTimer = 0.0f;
	bool bCollected = false;
};
