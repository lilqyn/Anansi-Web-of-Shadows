// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Zipline.generated.h"

class USplineComponent;
class USphereComponent;

/**
 * AZipline
 *
 * Two-point zipline the player can ride. Walk into the start trigger
 * to attach, then ride to the other end. Jump off at any point.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AZipline : public AActor
{
	GENERATED_BODY()

public:
	AZipline();

	virtual void Tick(float DeltaTime) override;

	/** End point offset from start. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Zipline")
	FVector EndOffset = FVector(1000, 0, -200);

	/** Speed of travel along the line. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Zipline", meta = (ClampMin = "100.0"))
	float ZipSpeed = 800.0f;

	UFUNCTION(BlueprintPure, Category = "Anansi|Zipline")
	bool IsRiding() const { return RidingCharacter != nullptr; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> StartTrigger;

	UFUNCTION()
	void OnStartOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY()
	TObjectPtr<ACharacter> RidingCharacter;

	float RideProgress = 0.0f;
	FVector StartWorldLocation;
	FVector EndWorldLocation;
};
