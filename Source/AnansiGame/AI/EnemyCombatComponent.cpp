// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/EnemyCombatComponent.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

int32 UEnemyCombatComponent::ActiveGroupAttackers = 0;

UEnemyCombatComponent::UEnemyCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Tick down cooldowns
	if (GlobalCooldownRemaining > 0.0f)
	{
		GlobalCooldownRemaining -= DeltaTime;
	}

	for (auto& Pair : AttackCooldowns)
	{
		if (Pair.Value > 0.0f)
		{
			Pair.Value -= DeltaTime;
		}
	}
}

// ---------------------------------------------------------------------------
// Attack interface
// ---------------------------------------------------------------------------

bool UEnemyCombatComponent::TryAttack(AActor* Target)
{
	if (!Target || bIsAttacking)
	{
		return false;
	}

	if (GlobalCooldownRemaining > 0.0f)
	{
		return false;
	}

	// Group coordination
	if (!CanAttackInGroup() && !RequestAttackSlot())
	{
		return false;
	}

	// Calculate distance
	const float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());

	// Check if target is airborne
	bool bTargetAirborne = false;
	if (const ACharacter* TargetChar = Cast<ACharacter>(Target))
	{
		bTargetAirborne = TargetChar->GetCharacterMovement() &&
			TargetChar->GetCharacterMovement()->IsFalling();
	}

	const FEnemyAttackPattern* SelectedPattern = SelectAttack(Distance, bTargetAirborne);
	if (!SelectedPattern)
	{
		return false;
	}

	ExecuteAttack(*SelectedPattern, Target);
	return true;
}

void UEnemyCombatComponent::InterruptAttack()
{
	if (!bIsAttacking)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar && OwnerChar->GetMesh() && OwnerChar->GetMesh()->GetAnimInstance())
	{
		OwnerChar->GetMesh()->GetAnimInstance()->StopAllMontages(0.2f);
	}

	OnAttackMontageEnded();
}

// ---------------------------------------------------------------------------
// Group coordination
// ---------------------------------------------------------------------------

bool UEnemyCombatComponent::CanAttackInGroup() const
{
	return bHasAttackSlot || (ActiveGroupAttackers < MaxGroupAttackers);
}

bool UEnemyCombatComponent::RequestAttackSlot()
{
	if (bHasAttackSlot)
	{
		return true;
	}

	if (ActiveGroupAttackers < MaxGroupAttackers)
	{
		ActiveGroupAttackers++;
		bHasAttackSlot = true;
		return true;
	}

	return false;
}

void UEnemyCombatComponent::ReleaseAttackSlot()
{
	if (bHasAttackSlot)
	{
		ActiveGroupAttackers = FMath::Max(0, ActiveGroupAttackers - 1);
		bHasAttackSlot = false;
	}
}

// ---------------------------------------------------------------------------
// Defensive
// ---------------------------------------------------------------------------

bool UEnemyCombatComponent::RollBlockChance() const
{
	return FMath::FRand() < BlockChance;
}

bool UEnemyCombatComponent::RollDodgeChance() const
{
	return FMath::FRand() < DodgeChance;
}

// ---------------------------------------------------------------------------
// Internal
// ---------------------------------------------------------------------------

const FEnemyAttackPattern* UEnemyCombatComponent::SelectAttack(float DistanceToTarget, bool bTargetAirborne) const
{
	// Build list of valid attacks
	TArray<const FEnemyAttackPattern*> ValidAttacks;
	float TotalWeight = 0.0f;

	for (const FEnemyAttackPattern& Pattern : AttackPatterns)
	{
		// Range check
		if (DistanceToTarget < Pattern.MinRange || DistanceToTarget > Pattern.MaxRange)
		{
			continue;
		}

		// Airborne requirement
		if (Pattern.bRequiresAirborneTarget && !bTargetAirborne)
		{
			continue;
		}

		// Per-attack cooldown
		const float* CooldownRemaining = AttackCooldowns.Find(Pattern.AttackName);
		if (CooldownRemaining && *CooldownRemaining > 0.0f)
		{
			continue;
		}

		// Montage required
		if (!Pattern.Montage)
		{
			continue;
		}

		ValidAttacks.Add(&Pattern);
		TotalWeight += Pattern.SelectionWeight;
	}

	if (ValidAttacks.Num() == 0)
	{
		return nullptr;
	}

	// Weighted random selection
	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	for (const FEnemyAttackPattern* Pattern : ValidAttacks)
	{
		RandomValue -= Pattern->SelectionWeight;
		if (RandomValue <= 0.0f)
		{
			return Pattern;
		}
	}

	return ValidAttacks.Last();
}

void UEnemyCombatComponent::ExecuteAttack(const FEnemyAttackPattern& Pattern, AActor* Target)
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar || !Pattern.Montage)
	{
		return;
	}

	// Face the target
	const FVector Direction = (Target->GetActorLocation() - OwnerChar->GetActorLocation()).GetSafeNormal();
	OwnerChar->SetActorRotation(Direction.Rotation());

	// Play attack montage
	UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		const float PlayRate = 1.0f;
		AnimInstance->Montage_Play(Pattern.Montage, PlayRate);

		// Bind montage end delegate
		FOnMontageEnded EndDelegate;
		EndDelegate.BindLambda([this](UAnimMontage*, bool)
		{
			OnAttackMontageEnded();
		});
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Pattern.Montage);
	}

	bIsAttacking = true;

	// Set per-attack cooldown
	AttackCooldowns.FindOrAdd(Pattern.AttackName) = Pattern.Cooldown;

	OnAttackStarted.Broadcast(Pattern);

	UE_LOG(LogAnansi, Verbose, TEXT("%s executing attack: %s"), *GetOwner()->GetName(), *Pattern.AttackName.ToString());
}

void UEnemyCombatComponent::OnAttackMontageEnded()
{
	bIsAttacking = false;
	GlobalCooldownRemaining = GlobalAttackCooldown;
	ReleaseAttackSlot();
	OnAttackEnded.Broadcast();
}
