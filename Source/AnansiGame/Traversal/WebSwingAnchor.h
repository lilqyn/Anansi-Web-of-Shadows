// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WebSwingAnchor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * AWebSwingAnchor
 *
 * Placeable anchor point for web-swinging. When the player presses
 * web-swing near this actor, the web attaches here instead of
 * tracing straight up. Provides a visible marker and a detection radius.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AWebSwingAnchor : public AActor
{
	GENERATED_BODY()

public:
	AWebSwingAnchor();

	/** The point where webs attach. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Traversal")
	FVector GetAnchorPoint() const { return GetActorLocation(); }

	/** Detection radius — player must be within this range to use this anchor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Traversal", meta = (ClampMin = "100.0"))
	float DetectionRadius = 1500.0f;

	/** Whether this anchor is currently active/usable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Traversal")
	bool bIsActive = true;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Traversal")
	TObjectPtr<USphereComponent> DetectionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Traversal")
	TObjectPtr<UStaticMeshComponent> AnchorMesh;
};
