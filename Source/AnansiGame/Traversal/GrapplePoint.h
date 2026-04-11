// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrapplePoint.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * AGrapplePoint
 *
 * Specific point in the level the player can grapple to using Silken Path.
 * When in range and looking at it, press T to zip directly to this location.
 * Distinct from WebSwingAnchor (swing) — this is a direct traversal zip.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AGrapplePoint : public AActor
{
	GENERATED_BODY()

public:
	AGrapplePoint();

	UFUNCTION(BlueprintPure, Category = "Anansi|Traversal")
	FVector GetLandingLocation() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Traversal", meta = (ClampMin = "500.0"))
	float GrappleRange = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Traversal")
	bool bIsActive = true;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> DetectionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PointMesh;
};
