// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/SpawnerPortal.h"
#include "AnansiGame.h"
#include "AI/EnemyGuard.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "UObject/ConstructorHelpers.h"

ASpawnerPortal::ASpawnerPortal()
{
	PrimaryActorTick.bCanEverTick = true;

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	RootComponent = PortalMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylMesh.Succeeded())
	{
		PortalMesh->SetStaticMesh(CylMesh.Object);
		PortalMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.1f));
	}
	PortalMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	PortalMesh->SetRenderCustomDepth(true);
	PortalMesh->SetCustomDepthStencilValue(4);

	EnemyClass = AEnemyGuard::StaticClass();
	CurrentHealth = PortalHealth;

	Tags.Add(FName("Enemy"));
}

void ASpawnerPortal::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = PortalHealth;
}

void ASpawnerPortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDestroyed) return;

	// Rotate for visual effect
	AddActorLocalRotation(FRotator(0, 120.0f * DeltaTime, 0));

	// Spawn on interval
	SpawnTimer += DeltaTime;
	if (SpawnTimer >= SpawnInterval && SpawnCount < MaxSpawns)
	{
		SpawnTimer = 0.0f;
		SpawnEnemy();
	}
}

float ASpawnerPortal::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bDestroyed) return 0.0f;

	CurrentHealth -= DamageAmount;

	if (CurrentHealth <= 0.0f)
	{
		bDestroyed = true;
		UE_LOG(LogAnansi, Log, TEXT("Spawner portal destroyed! (%d enemies spawned)"), SpawnCount);
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetLifeSpan(1.0f);
	}

	return DamageAmount;
}

void ASpawnerPortal::SpawnEnemy()
{
	if (!EnemyClass) return;

	const float Angle = FMath::FRandRange(0.0f, 360.0f);
	const FVector Offset(FMath::Cos(FMath::DegreesToRadians(Angle)) * 150.0f,
		FMath::Sin(FMath::DegreesToRadians(Angle)) * 150.0f, 50.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* Enemy = GetWorld()->SpawnActor<APawn>(EnemyClass, GetActorLocation() + Offset,
		FRotator::ZeroRotator, Params);

	if (Enemy)
	{
		Enemy->Tags.AddUnique(FName("Enemy"));
		SpawnCount++;
	}
}
