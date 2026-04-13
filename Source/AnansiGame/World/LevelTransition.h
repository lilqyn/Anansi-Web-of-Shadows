// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelTransition.generated.h"

class UBoxComponent;

/**
 * ALevelTransition
 *
 * Trigger volume that loads a new level when the player enters.
 * Supports fade-to-black transition.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ALevelTransition : public AActor
{
	GENERATED_BODY()

public:
	ALevelTransition();

	/** Name of the level to load. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Level")
	FName TargetLevelName;

	/** Fade duration before loading. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Level", meta = (ClampMin = "0.0"))
	float FadeDuration = 1.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> TriggerBox;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bTriggered = false;
	void DoLevelLoad();
};
