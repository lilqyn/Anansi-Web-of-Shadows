// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LootDrop.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ELootType : uint8
{
	Health		UMETA(DisplayName = "Health Orb"),
	WebEnergy	UMETA(DisplayName = "Web Energy Orb"),
	Stamina		UMETA(DisplayName = "Stamina Orb")
};

/**
 * ALootDrop
 *
 * Collectible orb dropped by defeated enemies. Auto-collected on
 * overlap with the player. Restores health, web energy, or stamina.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ALootDrop : public AActor
{
	GENERATED_BODY()

public:
	ALootDrop();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Loot")
	ELootType LootType = ELootType::Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Loot", meta = (ClampMin = "1.0"))
	float RestoreAmount = 20.0f;

	/** Spawn a random loot drop at the given location. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Loot", meta = (WorldContext = "WorldContext"))
	static ALootDrop* SpawnRandomLoot(UObject* WorldContext, FVector Location);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollectionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> LootMesh;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	float BobTimer = 0.0f;
	FVector SpawnWorldLocation;
	bool bCollected = false;
};
