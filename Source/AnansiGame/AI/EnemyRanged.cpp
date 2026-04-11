// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/EnemyRanged.h"
#include "AI/SimpleEnemyAI.h"
#include "AnansiGame.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyRanged::AEnemyRanged()
{
	PrimaryActorTick.bCanEverTick = true;

	EnemyType = EEnemyType::Zealot; // Using zealot archetype for ranged
	MaxHealth = 50.0f;
	BaseDamage = 8.0f;
	StaggerThreshold = 15.0f;

	RangedAI = CreateDefaultSubobject<USimpleEnemyAI>(TEXT("RangedAI"));
	RangedAI->DetectionRange = 2000.0f;
	RangedAI->AttackRange = PreferredRange;
	RangedAI->AttackCooldown = ShotCooldown;
	RangedAI->AttackDamage = 0.0f; // We handle damage ourselves
	RangedAI->ChaseSpeed = 250.0f;
}

void AEnemyRanged::BeginPlay()
{
	Super::BeginPlay();
	Tags.AddUnique(FName("Ranged"));
}

void AEnemyRanged::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	// Find player
	if (!Target.IsValid())
	{
		Target = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	}
	if (!Target.IsValid()) return;

	const float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

	// Maintain preferred distance
	if (DistToTarget < PreferredRange * 0.5f)
	{
		MaintainDistance(DeltaTime);
	}

	// Fire when in range and on cooldown
	if (DistToTarget <= PreferredRange * 1.2f && DistToTarget > 200.0f)
	{
		ShotTimer += DeltaTime;
		if (ShotTimer >= ShotCooldown)
		{
			ShotTimer = 0.0f;
			FireAtTarget();
		}

		// Face target
		const FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		const FRotator DesiredRot = Dir.Rotation();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), FRotator(0, DesiredRot.Yaw, 0), DeltaTime, 6.0f));
	}
}

void AEnemyRanged::FireAtTarget()
{
	if (!Target.IsValid()) return;

	// Line-of-sight check
	FHitResult LOSHit;
	FCollisionQueryParams LOSParams;
	LOSParams.AddIgnoredActor(this);

	const FVector Start = GetActorLocation() + FVector(0, 0, 50);
	const FVector End = Target->GetActorLocation() + FVector(0, 0, 50);

	if (GetWorld()->LineTraceSingleByChannel(LOSHit, Start, End, ECC_Visibility, LOSParams))
	{
		if (LOSHit.GetActor() != Target.Get()) return; // Blocked
	}

	// Deal damage
	FDamageEvent DamageEvent;
	Target->TakeDamage(RangedDamage, DamageEvent, GetController(), this);

	// Visual — draw a line from us to the target (simulated projectile)
#if ENABLE_DRAW_DEBUG
	DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 0.3f, 0, 2.0f);
#endif

	UE_LOG(LogAnansi, Verbose, TEXT("Ranged enemy fired at %s for %.0f damage"),
		*Target->GetName(), RangedDamage);
}

void AEnemyRanged::MaintainDistance(float DeltaTime)
{
	if (!Target.IsValid()) return;

	// Move away from player
	const FVector Away = (GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
	AddMovementInput(Away, 1.0f);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 350.0f; // Retreat speed
	}
}
