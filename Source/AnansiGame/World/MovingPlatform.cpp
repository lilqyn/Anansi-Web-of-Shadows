// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/MovingPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AMovingPlatform::AMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	RootComponent = PlatformMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PlatformMesh->SetStaticMesh(CubeMesh.Object);
		PlatformMesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 0.3f));
	}

	PlatformMesh->SetCollisionProfileName(TEXT("BlockAll"));
	PlatformMesh->SetMobility(EComponentMobility::Movable);
}

void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (StartLocation.IsZero())
	{
		StartLocation = GetActorLocation();
	}

	switch (MotionType)
	{
	case EPlatformMotion::Linear:
	{
		if (PauseTimer > 0.0f)
		{
			PauseTimer -= DeltaTime;
			return;
		}

		const FVector Target = bMovingForward ? StartLocation + EndOffset : StartLocation;
		const FVector Current = GetActorLocation();
		const FVector Direction = (Target - Current).GetSafeNormal();
		const float DistRemaining = FVector::Dist(Current, Target);

		if (DistRemaining < Speed * DeltaTime)
		{
			SetActorLocation(Target);
			bMovingForward = !bMovingForward;
			PauseTimer = PauseTime;
		}
		else
		{
			SetActorLocation(Current + Direction * Speed * DeltaTime);
		}
		break;
	}

	case EPlatformMotion::Circular:
	{
		MotionTimer += DeltaTime * Speed / Radius;
		const FVector Offset(
			FMath::Cos(MotionTimer) * Radius,
			FMath::Sin(MotionTimer) * Radius,
			0.0f);
		SetActorLocation(StartLocation + Offset);
		break;
	}

	case EPlatformMotion::Pendulum:
	{
		MotionTimer += DeltaTime * Speed / Radius;
		const float Swing = FMath::Sin(MotionTimer) * Radius;
		SetActorLocation(StartLocation + FVector(Swing, 0, 0));
		break;
	}
	}
}
