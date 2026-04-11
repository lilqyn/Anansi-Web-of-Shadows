// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Combat/Projectile.h"
#include "AnansiGame.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"

AAnansiProjectile::AAnansiProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionSphere->SetSphereRadius(10.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(false);
	RootComponent = CollisionSphere;

	CollisionSphere->OnComponentHit.AddDynamic(this, &AAnansiProjectile::OnHit);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		ProjectileMesh->SetStaticMesh(SphereMesh.Object);
		ProjectileMesh->SetRelativeScale3D(FVector(0.08f));
	}
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	ProjectileMovement->SetUpdatedComponent(CollisionSphere);
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	InitialLifeSpan = 5.0f;
	Tags.Add(FName("Projectile"));
}

void AAnansiProjectile::Fire(FVector Direction, float Speed)
{
	ProjectileMovement->Velocity = Direction.GetSafeNormal() * Speed;
	PreviousLocation = GetActorLocation();
}

void AAnansiProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if ENABLE_DRAW_DEBUG
	const FVector CurrentLoc = GetActorLocation();
	DrawDebugLine(GetWorld(), PreviousLocation, CurrentLoc, FColor::Yellow, false, 0.3f, 0, 1.5f);
	PreviousLocation = CurrentLoc;
#endif
}

void AAnansiProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		FDamageEvent DamageEvent;
		OtherActor->TakeDamage(Damage, DamageEvent, GetInstigatorController(), this);

		UE_LOG(LogAnansi, Verbose, TEXT("Projectile hit %s for %.0f damage"), *OtherActor->GetName(), Damage);
	}

	Destroy();
}
