// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/AllyCompanion.h"
#include "AI/EnemyBase.h"
#include "Combat/MeleeDamageDealer.h"
#include "AnansiGame.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AAllyCompanion::AAllyCompanion()
{
	PrimaryActorTick.bCanEverTick = true;

	MeleeDamage = CreateDefaultSubobject<UMeleeDamageDealer>(TEXT("MeleeDamage"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 540, 0);
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;

	CurrentHealth = MaxHealth;
	Tags.Add(FName("Ally"));
}

void AAllyCompanion::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	if (MeleeDamage)
	{
		MeleeDamage->BaseDamage = AttackDamage;
		MeleeDamage->AttackRange = 200.0f;
	}
}

void AAllyCompanion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	if (AttackTimer > 0.0f) AttackTimer -= DeltaTime;

	FindTarget();

	if (CurrentTarget.IsValid())
	{
		EngageTarget(DeltaTime);
	}
	else
	{
		FollowPlayer(DeltaTime);
	}
}

void AAllyCompanion::FindTarget()
{
	// Clear invalid target
	if (CurrentTarget.IsValid())
	{
		if (AEnemyBase* EBase = Cast<AEnemyBase>(CurrentTarget.Get()))
		{
			if (EBase->IsDead())
			{
				CurrentTarget.Reset();
			}
		}
	}

	if (CurrentTarget.IsValid())
	{
		// Check still in range
		if (FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation()) > EngageRange * 1.5f)
		{
			CurrentTarget.Reset();
		}
		return;
	}

	// Find nearest enemy
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	float BestDist = EngageRange;
	AActor* Best = nullptr;
	for (AActor* E : Enemies)
	{
		if (!E) continue;
		AEnemyBase* EBase = Cast<AEnemyBase>(E);
		if (EBase && EBase->IsDead()) continue;

		const float Dist = FVector::Dist(GetActorLocation(), E->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			Best = E;
		}
	}

	if (Best) CurrentTarget = Best;
}

void AAllyCompanion::FollowPlayer(float DeltaTime)
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player) return;

	const float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	if (Dist < FollowDistance) return;

	const FVector Dir = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	AddMovementInput(Dir, 1.0f);
}

void AAllyCompanion::EngageTarget(float DeltaTime)
{
	if (!CurrentTarget.IsValid()) return;

	const float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	const FVector Dir = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();

	// Face target
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), Dir.Rotation(), DeltaTime, 8.0f));

	// Move closer or attack
	if (Dist > 180.0f)
	{
		AddMovementInput(Dir, 1.0f);
	}
	else if (AttackTimer <= 0.0f && MeleeDamage)
	{
		MeleeDamage->FireAttack(AttackDamage);
		AttackTimer = AttackCooldown;
	}
}

float AAllyCompanion::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;

	CurrentHealth -= DamageAmount;
	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		UE_LOG(LogAnansi, Log, TEXT("Ally companion died"));
		SetLifeSpan(3.0f);
	}

	return DamageAmount;
}
