// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerupPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EPowerupType : uint8
{
	SpeedBoost		UMETA(DisplayName = "Speed Boost"),
	DamageBoost		UMETA(DisplayName = "Damage Boost"),
	Shield			UMETA(DisplayName = "Temporary Shield")
};

/**
 * APowerupPickup
 *
 * Temporary powerup that grants a buff for a duration.
 * Bobs and rotates, collected on overlap.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API APowerupPickup : public AActor
{
	GENERATED_BODY()

public:
	APowerupPickup();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Powerup")
	EPowerupType PowerupType = EPowerupType::SpeedBoost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Powerup", meta = (ClampMin = "1.0"))
	float Duration = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Powerup", meta = (ClampMin = "1.0"))
	float BoostMultiplier = 1.5f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollectionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	FVector SpawnLoc;
	float BobTimer = 0.0f;
	bool bCollected = false;
};
