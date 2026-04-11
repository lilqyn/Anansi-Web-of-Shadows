// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/EnemyGuard.h"
#include "AI/SimpleEnemyAI.h"
#include "AnansiGame.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyGuard::AEnemyGuard()
{
	PrimaryActorTick.bCanEverTick = true;

	EnemyType = EEnemyType::Guard;
	MaxHealth = 80.0f;
	BaseDamage = 12.0f;
	AttackRange = 180.0f;
	StaggerThreshold = 20.0f;

	// Attach AI component
	EnemyAI = CreateDefaultSubobject<USimpleEnemyAI>(TEXT("EnemyAI"));
	EnemyAI->AttackDamage = BaseDamage;
	EnemyAI->AttackRange = AttackRange;
}

void AEnemyGuard::BeginPlay()
{
	Super::BeginPlay();

	CurrentShieldStamina = MaxShieldStamina;

	switch (GuardVariant)
	{
	case EGuardVariant::Melee:
		InitMeleeVariant();
		break;
	case EGuardVariant::Shield:
		InitShieldVariant();
		break;
	}

	Tags.AddUnique(FName("Enemy"));
	Tags.AddUnique(FName("Guard"));
}

void AEnemyGuard::InitMeleeVariant()
{
	// Melee guards: standard stats, faster movement
	GetCharacterMovement()->MaxWalkSpeed = 450.0f;
	AttackSpeed = 1.2f;
}

void AEnemyGuard::InitShieldVariant()
{
	// Shield guards: tankier, slower, higher damage
	MaxHealth = 120.0f;
	BaseDamage = 18.0f;
	GetCharacterMovement()->MaxWalkSpeed = 350.0f;
	AttackSpeed = 0.8f;
	StaggerThreshold = 35.0f;

	// Start with shield raised
	SetBlocking(true);
}

float AEnemyGuard::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead())
	{
		return 0.0f;
	}

	float FinalDamage = DamageAmount;

	// Shield damage reduction for frontal attacks
	if (bIsBlocking && DamageCauser && GuardVariant == EGuardVariant::Shield)
	{
		const FVector AttackDir = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();

		if (IsAttackBlockedByShield(AttackDir))
		{
			FinalDamage *= ShieldDamageReduction;
			CurrentShieldStamina -= BlockStaminaCost;

			// Play block impact animation
			if (BlockImpactMontage)
			{
				if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
				{
					AnimInstance->Montage_Play(BlockImpactMontage);
				}
			}

			// Shield break when stamina depleted
			if (CurrentShieldStamina <= 0.0f)
			{
				CurrentShieldStamina = 0.0f;
				SetBlocking(false);

				if (ShieldBreakMontage)
				{
					if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
					{
						AnimInstance->Montage_Play(ShieldBreakMontage);
					}
				}

				UE_LOG(LogAnansi, Log, TEXT("Guard %s: Shield broken!"), *GetName());
			}

			UE_LOG(LogAnansi, Verbose, TEXT("Guard %s: Blocked hit (%.1f -> %.1f, stamina: %.0f)"),
				*GetName(), DamageAmount, FinalDamage, CurrentShieldStamina);
		}
	}

	return Super::TakeDamage(FinalDamage, DamageEvent, EventInstigator, DamageCauser);
}

void AEnemyGuard::SetBlocking(bool bBlock)
{
	if (GuardVariant != EGuardVariant::Shield)
	{
		return;
	}

	bIsBlocking = bBlock && CurrentShieldStamina > 0.0f;
}

bool AEnemyGuard::IsAttackBlockedByShield(const FVector& AttackDirection) const
{
	if (!bIsBlocking)
	{
		return false;
	}

	const FVector Forward = GetActorForwardVector();
	// AttackDirection points from attacker towards us, so we negate for dot product
	const float Dot = FVector::DotProduct(Forward, -AttackDirection);
	const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(ShieldConeHalfAngle));

	return Dot >= CosThreshold;
}
