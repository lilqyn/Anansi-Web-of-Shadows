// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WallRunVolume.generated.h"

class UStaticMeshComponent;

/**
 * AWallRunVolume
 *
 * Placeable wall surface for wall-run testing. Self-contained with a
 * visible mesh and proper collision. Place these in the test arena
 * to provide wall-run surfaces without needing BSP or imported geometry.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AWallRunVolume : public AActor
{
	GENERATED_BODY()

public:
	AWallRunVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Traversal")
	TObjectPtr<UStaticMeshComponent> WallMesh;

	/** Height of the wall in units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Traversal", meta = (ClampMin = "100.0"))
	float WallHeight = 500.0f;

	/** Width of the wall in units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Traversal", meta = (ClampMin = "100.0"))
	float WallWidth = 800.0f;

	virtual void OnConstruction(const FTransform& Transform) override;
};
