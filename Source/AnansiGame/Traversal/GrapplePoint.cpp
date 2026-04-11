// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Traversal/GrapplePoint.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AGrapplePoint::AGrapplePoint()
{
	PrimaryActorTick.bCanEverTick = false;

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetSphereRadius(GrappleRange);
	DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DetectionSphere->SetGenerateOverlapEvents(true);
	DetectionSphere->SetHiddenInGame(false);
	DetectionSphere->ShapeColor = FColor::Green;
	RootComponent = DetectionSphere;

	PointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PointMesh"));
	PointMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DiamondMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (DiamondMesh.Succeeded())
	{
		PointMesh->SetStaticMesh(DiamondMesh.Object);
		PointMesh->SetRelativeScale3D(FVector(0.2f));
		PointMesh->SetRelativeRotation(FRotator(45, 45, 0));
	}
	PointMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PointMesh->SetRenderCustomDepth(true);
	PointMesh->SetCustomDepthStencilValue(2);

	Tags.Add(FName("GrapplePoint"));
}

FVector AGrapplePoint::GetLandingLocation() const
{
	return GetActorLocation();
}
