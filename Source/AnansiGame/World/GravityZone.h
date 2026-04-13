// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GravityZone.generated.h"

class UBoxComponent;

/**
 * AGravityZone
 *
 * Volume that modifies gravity for any character inside it.
 * Use for low-gravity traversal puzzles or high-gravity trap zones.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AGravityZone : public AActor
{
	GENERATED_BODY()

public:
	AGravityZone();

	/** Gravity scale while inside (0.1 = moon gravity, 3.0 = heavy). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Gravity", meta = (ClampMin = "0.0"))
	float GravityScale = 0.3f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> GravityVolume;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* Comp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* Comp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
