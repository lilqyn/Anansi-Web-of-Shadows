// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Traversal/WebSwingAnchor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AWebSwingAnchor::AWebSwingAnchor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Visual marker (small sphere)
	AnchorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnchorMesh"));
	RootComponent = AnchorMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		AnchorMesh->SetStaticMesh(SphereMesh.Object);
	}

	AnchorMesh->SetWorldScale3D(FVector(0.3f));
	AnchorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Detection sphere
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(DetectionRadius);
	DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DetectionSphere->SetGenerateOverlapEvents(true);
	DetectionSphere->SetHiddenInGame(false);
	DetectionSphere->ShapeColor = FColor::Cyan;

	Tags.Add(FName("WebAnchor"));
}
