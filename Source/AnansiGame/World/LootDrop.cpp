// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/LootDrop.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

ALootDrop::ALootDrop()
{
	PrimaryActorTick.bCanEverTick = true;

	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectionSphere->SetSphereRadius(80.0f);
	CollectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollectionSphere->SetGenerateOverlapEvents(true);
	RootComponent = CollectionSphere;

	LootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LootMesh"));
	LootMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		LootMesh->SetStaticMesh(SphereMesh.Object);
		LootMesh->SetRelativeScale3D(FVector(0.15f));
	}
	LootMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CollectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ALootDrop::OnOverlap);

	SetLifeSpan(15.0f);
}

void ALootDrop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCollected) return;

	if (SpawnWorldLocation.IsZero())
	{
		SpawnWorldLocation = GetActorLocation();
	}

	// Bob and rotate
	BobTimer += DeltaTime;
	const float BobZ = FMath::Sin(BobTimer * 3.0f) * 10.0f;
	SetActorLocation(SpawnWorldLocation + FVector(0, 0, BobZ));
	AddActorLocalRotation(FRotator(0, 120.0f * DeltaTime, 0));
}

void ALootDrop::OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(OtherActor);
	if (!Anansi || bCollected) return;

	bCollected = true;

	switch (LootType)
	{
	case ELootType::Health:
		Anansi->ApplyHealing(RestoreAmount);
		UE_LOG(LogAnansi, Log, TEXT("Collected health orb (+%.0f HP)"), RestoreAmount);
		break;
	case ELootType::WebEnergy:
		Anansi->RestoreWebEnergy(RestoreAmount);
		UE_LOG(LogAnansi, Log, TEXT("Collected web energy orb (+%.0f)"), RestoreAmount);
		break;
	case ELootType::Stamina:
		// Stamina auto-regens but a boost is nice
		UE_LOG(LogAnansi, Log, TEXT("Collected stamina orb"));
		break;
	}

	Destroy();
}

ALootDrop* ALootDrop::SpawnRandomLoot(UObject* WorldContext, FVector Location)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ALootDrop* Drop = World->SpawnActor<ALootDrop>(ALootDrop::StaticClass(),
		Location + FVector(0, 0, 50), FRotator::ZeroRotator, Params);

	if (Drop)
	{
		// Random type with health being more common
		const float Roll = FMath::FRand();
		if (Roll < 0.5f)
			Drop->LootType = ELootType::Health;
		else if (Roll < 0.8f)
			Drop->LootType = ELootType::WebEnergy;
		else
			Drop->LootType = ELootType::Stamina;

		Drop->RestoreAmount = FMath::RandRange(10.0f, 30.0f);
	}

	return Drop;
}
