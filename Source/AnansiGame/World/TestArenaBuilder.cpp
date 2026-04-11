// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/TestArenaBuilder.h"
#include "AnansiGame.h"
#include "Traversal/WebSwingAnchor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

// Store mesh references for spawning
static UStaticMesh* GCubeMesh = nullptr;
static UMaterialInterface* GFloorMat = nullptr;
static UMaterialInterface* GWallMat = nullptr;
static UMaterialInterface* GPlatformMat = nullptr;

ATestArenaBuilder::ATestArenaBuilder()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Cache engine meshes and materials
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		GCubeMesh = CubeFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FloorMatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (FloorMatFinder.Succeeded())
	{
		GFloorMat = FloorMatFinder.Object;
		GWallMat = FloorMatFinder.Object;
		GPlatformMat = FloorMatFinder.Object;
	}
}

void ATestArenaBuilder::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bAutoBuilt)
	{
		BuildArena();
	}
}

void ATestArenaBuilder::BuildArena()
{
	ClearArena();

	SpawnFloor();
	SpawnPerimeterWalls();
	SpawnWallRunSurfaces();
	SpawnPlatforms();
	SpawnWebAnchors();
	SpawnRamps();

	UE_LOG(LogAnansi, Log, TEXT("TestArenaBuilder: Arena built with %d actors"), SpawnedActors.Num());
}

void ATestArenaBuilder::ClearArena()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();
}

// ---------------------------------------------------------------------------
// Arena construction
// ---------------------------------------------------------------------------

void ATestArenaBuilder::SpawnFloor()
{
	const FVector Origin = GetActorLocation();
	const float HalfSize = ArenaSize * 0.5f;

	// Main floor — scale cube to flat plane
	// Cube is 100x100x100, so ScaleXY = ArenaSize/100, ScaleZ thin
	AActor* Floor = SpawnCube(
		Origin + FVector(0, 0, -25),
		FVector(ArenaSize / 100.0f, ArenaSize / 100.0f, 0.5f));

	if (Floor)
	{
		Floor->SetActorLabel(TEXT("Arena_Floor"));
		Floor->Tags.Add(FName("ArenaFloor"));
	}
}

void ATestArenaBuilder::SpawnPerimeterWalls()
{
	const FVector Origin = GetActorLocation();
	const float Half = ArenaSize * 0.5f;
	const float WallThickness = 25.0f;
	const float WallZ = WallHeight * 0.5f;

	// North wall
	SpawnCube(
		Origin + FVector(Half, 0, WallZ),
		FVector(WallThickness / 100.0f, ArenaSize / 100.0f, WallHeight / 100.0f));

	// South wall
	SpawnCube(
		Origin + FVector(-Half, 0, WallZ),
		FVector(WallThickness / 100.0f, ArenaSize / 100.0f, WallHeight / 100.0f));

	// East wall
	SpawnCube(
		Origin + FVector(0, Half, WallZ),
		FVector(ArenaSize / 100.0f, WallThickness / 100.0f, WallHeight / 100.0f));

	// West wall
	SpawnCube(
		Origin + FVector(0, -Half, WallZ),
		FVector(ArenaSize / 100.0f, WallThickness / 100.0f, WallHeight / 100.0f));
}

void ATestArenaBuilder::SpawnWallRunSurfaces()
{
	const FVector Origin = GetActorLocation();
	const float Radius = ArenaSize * 0.3f;

	for (int32 i = 0; i < WallRunSurfaceCount; ++i)
	{
		const float Angle = (static_cast<float>(i) / WallRunSurfaceCount) * 360.0f;
		const float Rad = FMath::DegreesToRadians(Angle);

		const FVector Pos = Origin + FVector(
			FMath::Cos(Rad) * Radius,
			FMath::Sin(Rad) * Radius,
			WallHeight * 0.5f);

		// Wall faces inward towards center
		const FRotator Rot(0, Angle + 90.0f, 0);

		// Tall thin wall for wall-running
		AActor* Wall = SpawnCube(Pos,
			FVector(0.25f, 8.0f, WallHeight / 100.0f),
			Rot);

		if (Wall)
		{
			Wall->Tags.Add(FName("WallRunSurface"));
		}
	}
}

void ATestArenaBuilder::SpawnPlatforms()
{
	const FVector Origin = GetActorLocation();

	// Platforms at varying heights for jump/traversal testing
	struct FPlatformDef
	{
		FVector Offset;
		FVector Scale;
	};

	const TArray<FPlatformDef> Defs = {
		// Low platform — easy jump
		{ FVector(800, 800, 100),    FVector(3.0f, 3.0f, 0.5f) },
		// Medium platform — needs double jump or wall-run
		{ FVector(-800, 800, 300),   FVector(3.0f, 3.0f, 0.5f) },
		// High platform — needs web-swing
		{ FVector(800, -800, 600),   FVector(4.0f, 4.0f, 0.5f) },
		// Tall pillar — sniper perch
		{ FVector(-800, -800, 400),  FVector(2.0f, 2.0f, 0.5f) },
	};

	const int32 Count = FMath::Min(PlatformCount, Defs.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		AActor* Plat = SpawnCube(
			Origin + Defs[i].Offset,
			Defs[i].Scale);

		if (Plat)
		{
			Plat->SetActorLabel(FString::Printf(TEXT("Arena_Platform_%d"), i));

			// Add a pillar underneath
			const float PillarHeight = Defs[i].Offset.Z;
			SpawnCube(
				Origin + FVector(Defs[i].Offset.X, Defs[i].Offset.Y, PillarHeight * 0.5f),
				FVector(0.5f, 0.5f, PillarHeight / 100.0f));
		}
	}
}

void ATestArenaBuilder::SpawnWebAnchors()
{
	const FVector Origin = GetActorLocation();
	const float Radius = ArenaSize * 0.25f;
	const float AnchorHeight = 1500.0f;

	for (int32 i = 0; i < WebAnchorCount; ++i)
	{
		const float Angle = (static_cast<float>(i) / WebAnchorCount) * 360.0f;
		const float Rad = FMath::DegreesToRadians(Angle);

		// Alternate heights for variety
		const float Height = AnchorHeight + (i % 2 == 0 ? 0.0f : 500.0f);

		const FVector Pos = Origin + FVector(
			FMath::Cos(Rad) * Radius,
			FMath::Sin(Rad) * Radius,
			Height);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AWebSwingAnchor* Anchor = GetWorld()->SpawnActor<AWebSwingAnchor>(
			AWebSwingAnchor::StaticClass(), Pos, FRotator::ZeroRotator, Params);

		if (Anchor)
		{
			Anchor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
			SpawnedActors.Add(Anchor);
		}
	}
}

void ATestArenaBuilder::SpawnRamps()
{
	const FVector Origin = GetActorLocation();

	// Ramp leading up to the low platform
	SpawnCube(
		Origin + FVector(500, 800, 50),
		FVector(4.0f, 2.0f, 0.25f),
		FRotator(15.0f, 0, 0));

	// Ramp on the opposite side
	SpawnCube(
		Origin + FVector(-500, -800, 50),
		FVector(4.0f, 2.0f, 0.25f),
		FRotator(-15.0f, 0, 0));
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

AActor* ATestArenaBuilder::SpawnCube(const FVector& Location, const FVector& Scale, const FRotator& Rotation)
{
	if (!GCubeMesh || !GetWorld())
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* CubeActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation, Params);
	if (!CubeActor)
	{
		return nullptr;
	}

	UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(CubeActor, TEXT("Mesh"));
	MeshComp->SetStaticMesh(GCubeMesh);
	MeshComp->SetWorldScale3D(Scale);
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));
	MeshComp->SetMobility(EComponentMobility::Static);

	if (GFloorMat)
	{
		MeshComp->SetMaterial(0, GFloorMat);
	}

	CubeActor->SetRootComponent(MeshComp);
	MeshComp->RegisterComponent();

	CubeActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedActors.Add(CubeActor);

	return CubeActor;
}
