// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestArenaBuilder.generated.h"

/**
 * ATestArenaBuilder
 *
 * Drop this actor into any level and it builds a complete test arena:
 * - Large ground plane
 * - Perimeter walls
 * - Wall-run surfaces at varying angles
 * - Elevated platforms for jump/fall testing
 * - Web-swing anchors overhead
 * - Ramps and stairs
 *
 * All geometry uses engine basic shapes — no imported assets required.
 * Delete this actor to remove all spawned geometry.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ATestArenaBuilder : public AActor
{
	GENERATED_BODY()

public:
	ATestArenaBuilder();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** Rebuild the arena. Call after changing parameters. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Anansi|Arena")
	void BuildArena();

	/** Remove all spawned arena geometry. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Anansi|Arena")
	void ClearArena();

	// -- Configuration -------------------------------------------------------

	/** Size of the arena ground plane (square, in units). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Arena", meta = (ClampMin = "1000.0"))
	float ArenaSize = 6000.0f;

	/** Height of the perimeter walls. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Arena", meta = (ClampMin = "200.0"))
	float WallHeight = 600.0f;

	/** Number of wall-run surfaces to place. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Arena", meta = (ClampMin = "0", ClampMax = "20"))
	int32 WallRunSurfaceCount = 6;

	/** Number of elevated platforms. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Arena", meta = (ClampMin = "0", ClampMax = "10"))
	int32 PlatformCount = 4;

	/** Number of web-swing anchors overhead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Arena", meta = (ClampMin = "0", ClampMax = "20"))
	int32 WebAnchorCount = 8;

	/** Whether to auto-build on construction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Arena")
	bool bAutoBuilt = true;

private:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedActors;

	void SpawnFloor();
	void SpawnPerimeterWalls();
	void SpawnWallRunSurfaces();
	void SpawnPlatforms();
	void SpawnWebAnchors();
	void SpawnRamps();

	AActor* SpawnCube(const FVector& Location, const FVector& Scale, const FRotator& Rotation = FRotator::ZeroRotator);
};
