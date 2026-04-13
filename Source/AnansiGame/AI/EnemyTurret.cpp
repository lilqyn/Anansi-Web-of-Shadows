// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/EnemyTurret.h"
#include "AnansiGame.h"
#include "Combat/Projectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AEnemyTurret::AEnemyTurret()
{
	PrimaryActorTick.bCanEverTick = true;

	EnemyType = EEnemyType::Guard;
	MaxHealth = 60.0f;
	StaggerThreshold = 100.0f; // Very hard to stagger
	bRagdollOnDeath = false;

	// Turrets don't move
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}
}

void AEnemyTurret::BeginPlay()
{
	Super::BeginPlay();
	Tags.AddUnique(FName("Turret"));
}

void AEnemyTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	FindTarget();

	if (!Target.IsValid()) return;

	const float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	if (Dist > DetectionRange)
	{
		Target.Reset();
		return;
	}

	AimAtTarget(DeltaTime);

	if (HasLineOfSight())
	{
		FireTimer += DeltaTime;
		if (FireTimer >= FireRate)
		{
			FireTimer = 0.0f;
			Fire();
		}

		// Detection indicator
		SetAwarenessState(EEnemyAwareness::Combat);
	}
}

void AEnemyTurret::FindTarget()
{
	if (Target.IsValid()) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (Player)
	{
		const float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
		if (Dist <= DetectionRange)
		{
			Target = Player;
		}
	}
}

void AEnemyTurret::AimAtTarget(float DeltaTime)
{
	if (!Target.IsValid()) return;

	const FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const FRotator DesiredRot = Dir.Rotation();
	const FRotator CurrentRot = GetActorRotation();
	const FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, DesiredRot, DeltaTime, TurnSpeed);

	SetActorRotation(FRotator(0, NewRot.Yaw, 0));
}

bool AEnemyTurret::HasLineOfSight() const
{
	if (!Target.IsValid()) return false;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const FVector Start = GetActorLocation() + FVector(0, 0, 50);
	const FVector End = Target->GetActorLocation() + FVector(0, 0, 50);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.GetActor() == Target.Get();
	}

	return true; // No obstruction
}

void AEnemyTurret::Fire()
{
	if (!Target.IsValid()) return;

	const FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 80.0f + FVector(0, 0, 50);
	const FVector Dir = (Target->GetActorLocation() + FVector(0, 0, 50) - SpawnLoc).GetSafeNormal();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Instigator = this;

	AAnansiProjectile* Proj = GetWorld()->SpawnActor<AAnansiProjectile>(
		AAnansiProjectile::StaticClass(), SpawnLoc, Dir.Rotation(), Params);

	if (Proj)
	{
		Proj->Damage = ProjectileDamage;
		Proj->SetInstigator(this);
		Proj->Fire(Dir, ProjectileSpeed);
	}

#if ENABLE_DRAW_DEBUG
	DrawDebugPoint(GetWorld(), SpawnLoc, 10.0f, FColor::Red, false, 0.2f);
#endif
}
