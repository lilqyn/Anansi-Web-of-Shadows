// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/BreakableObject.h"
#include "World/LootDrop.h"
#include "AnansiGame.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "UObject/ConstructorHelpers.h"

ABreakableObject::ABreakableObject()
{
	PrimaryActorTick.bCanEverTick = false;

	ObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectMesh"));
	RootComponent = ObjectMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		ObjectMesh->SetStaticMesh(CubeMesh.Object);
		ObjectMesh->SetRelativeScale3D(FVector(ObjectScale));
	}

	ObjectMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ObjectMesh->SetSimulatePhysics(false);

	CurrentHealth = MaxHealth;
}

float ABreakableObject::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsBroken) return 0.0f;

	CurrentHealth -= DamageAmount;

	// Visual feedback — slight scale pulse
	if (ObjectMesh)
	{
		const FVector OrigScale = FVector(ObjectScale);
		ObjectMesh->SetRelativeScale3D(OrigScale * 0.9f);

		FTimerHandle ResetTimer;
		GetWorldTimerManager().SetTimer(ResetTimer, [this, OrigScale]()
		{
			if (ObjectMesh && !bIsBroken)
				ObjectMesh->SetRelativeScale3D(OrigScale);
		}, 0.1f, false);
	}

	if (CurrentHealth <= 0.0f)
	{
		Break(DamageCauser);
	}

	return DamageAmount;
}

void ABreakableObject::Break(AActor* Breaker)
{
	bIsBroken = true;
	OnBroken.Broadcast(this);

	if (bDropsLoot)
	{
		ALootDrop::SpawnRandomLoot(this, GetActorLocation());
	}

	UE_LOG(LogAnansi, Log, TEXT("Breakable %s destroyed"), *GetName());

	// Disappear
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetLifeSpan(0.5f);
}
