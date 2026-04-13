// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BouncePad.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * ABouncePad
 *
 * Launches any character that steps on it upward. Configurable
 * launch force and direction. Visual indicator on the pad.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ABouncePad : public AActor
{
	GENERATED_BODY()

public:
	ABouncePad();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|BouncePad")
	FVector LaunchVelocity = FVector(0, 0, 1500);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|BouncePad", meta = (ClampMin = "0.0"))
	float Cooldown = 0.5f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PadMesh;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	float CooldownTimer = 0.0f;
};
