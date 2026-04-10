// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Traversal/WebSwingPoint.h"
#include "AnansiGame.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AWebSwingPoint::AWebSwingPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// Detection volume
	DetectionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionVolume"));
	DetectionVolume->SetSphereRadius(AutoDetectionRadius);
	DetectionVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DetectionVolume->SetGenerateOverlapEvents(true);
	RootComponent = DetectionVolume;

	// Visual mesh
	PointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PointMesh"));
	PointMesh->SetupAttachment(RootComponent);
	PointMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PointMesh->SetRelativeScale3D(FVector(0.3f));
}

void AWebSwingPoint::SetActive(bool bActive)
{
	bIsActive = bActive;
	if (PointMesh)
	{
		PointMesh->SetVisibility(bActive);
	}
}

bool AWebSwingPoint::CanAttachFrom(const FVector& CharacterLocation) const
{
	if (!bIsActive)
	{
		return false;
	}

	const FVector PointLocation = GetActorLocation();

	// Must be below the swing point
	if (CharacterLocation.Z >= PointLocation.Z)
	{
		return false;
	}

	// Distance check
	const float Distance = FVector::Dist(CharacterLocation, PointLocation);
	if (Distance > SwingRadius)
	{
		return false;
	}

	// Entry angle check
	const FVector ToCharacter = (CharacterLocation - PointLocation).GetSafeNormal();
	const float AngleFromVertical = FMath::RadiansToDegrees(FMath::Acos(FMath::Abs(ToCharacter.Z)));
	if (AngleFromVertical > MaxEntryAngle)
	{
		return false;
	}

	return true;
}

FVector AWebSwingPoint::CalculateReleaseVelocity(float CurrentAngle, float AngularVelocity) const
{
	// Tangential velocity from the pendulum
	const float TangentialSpeed = AngularVelocity * SwingRadius;

	// Tangent direction perpendicular to the rope at the current angle
	const FVector TangentDirection(FMath::Cos(CurrentAngle), 0.0f, FMath::Sin(CurrentAngle));

	// Add a vertical boost for momentum preservation
	FVector ReleaseVel = TangentDirection * TangentialSpeed;
	if (ReleaseVel.Z < 0.0f)
	{
		ReleaseVel.Z *= 0.5f; // Reduce downward velocity on release
	}

	return ReleaseVel;
}
