// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/DestructibleWall.h"
#include "AnansiGame.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ADestructibleWall::ADestructibleWall()
{
	PrimaryActorTick.bCanEverTick = false;

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = WallMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		WallMesh->SetStaticMesh(CubeMesh.Object);
		WallMesh->SetRelativeScale3D(WallScale);
	}

	WallMesh->SetCollisionProfileName(TEXT("BlockAll"));
	WallMesh->SetRenderCustomDepth(true);
	WallMesh->SetCustomDepthStencilValue(4); // Gold highlight for Spider Sense

	CurrentHealth = MaxHealth;
	Tags.Add(FName("Destructible"));
}

float ADestructibleWall::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsBroken) return 0.0f;

	// Only accept hits above threshold (requires heavy attacks)
	if (DamageAmount < MinDamageThreshold)
	{
		UE_LOG(LogAnansi, Verbose, TEXT("Wall resists weak hit (%.0f < %.0f)"),
			DamageAmount, MinDamageThreshold);
		return 0.0f;
	}

	CurrentHealth -= DamageAmount;

	// Crack visual — scale jitter
	if (WallMesh)
	{
		const FVector Jitter(
			FMath::FRandRange(-0.02f, 0.02f),
			FMath::FRandRange(-0.02f, 0.02f),
			0.0f);
		WallMesh->SetRelativeScale3D(WallScale + Jitter);

		FTimerHandle ResetTimer;
		GetWorldTimerManager().SetTimer(ResetTimer, [this]()
		{
			if (WallMesh && !bIsBroken)
				WallMesh->SetRelativeScale3D(WallScale);
		}, 0.15f, false);
	}

	if (CurrentHealth <= 0.0f)
	{
		Break();
	}

	return DamageAmount;
}

void ADestructibleWall::Break()
{
	bIsBroken = true;

	// Fragments flying outward
	if (WallMesh)
	{
		WallMesh->SetSimulatePhysics(true);
		WallMesh->AddImpulse(FVector(0, 0, 200), NAME_None, true);
	}

	UE_LOG(LogAnansi, Log, TEXT("Destructible wall broken!"));

	// Let it fall for a moment then destroy
	SetLifeSpan(2.0f);

	// Disable collision so player can pass through
	SetActorEnableCollision(false);
}
