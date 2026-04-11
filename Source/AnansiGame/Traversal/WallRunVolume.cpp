// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Traversal/WallRunVolume.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AWallRunVolume::AWallRunVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = WallMesh;

	// Use a default cube mesh scaled to wall proportions.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		WallMesh->SetStaticMesh(CubeMesh.Object);
	}

	WallMesh->SetCollisionProfileName(TEXT("BlockAll"));
	WallMesh->SetMobility(EComponentMobility::Static);

	Tags.Add(FName("WallRunSurface"));
}

void AWallRunVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Scale the cube to match the wall dimensions.
	// Default cube is 100x100x100, so we scale accordingly.
	const float ScaleX = 25.0f / 100.0f;       // Thin wall (25 units thick)
	const float ScaleY = WallWidth / 100.0f;
	const float ScaleZ = WallHeight / 100.0f;

	WallMesh->SetWorldScale3D(FVector(ScaleX, ScaleY, ScaleZ));
}
