// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/SimpleEnemyAI.h"
#include "AnansiGame.h"
#include "AI/EnemyBase.h"
#include "Player/AnansiCharacter.h"
#include "UI/AnansiDevHUD.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

USimpleEnemyAI::USimpleEnemyAI()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USimpleEnemyAI::BeginPlay()
{
	Super::BeginPlay();

	// Listen for damage so we can stagger
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &USimpleEnemyAI::OnOwnerTakeDamage);
		SpawnLocation = Owner->GetActorLocation();
		PickNewPatrolTarget();
	}
}

void USimpleEnemyAI::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check if owner is dead
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetOwner()))
	{
		if (Enemy->IsDead())
		{
			AIState = ESimpleAIState::Dead;
			SetComponentTickEnabled(false);
			return;
		}
	}

	// Cooldown timer
	if (AttackCooldownRemaining > 0.0f)
	{
		AttackCooldownRemaining -= DeltaTime;
	}

	switch (AIState)
	{
	case ESimpleAIState::Idle:    TickIdle(DeltaTime);    break;
	case ESimpleAIState::Chase:   TickChase(DeltaTime);   break;
	case ESimpleAIState::Attack:  TickAttack(DeltaTime);  break;
	case ESimpleAIState::Stagger: TickStagger(DeltaTime); break;
	default: break;
	}
}

// ---------------------------------------------------------------------------
// State ticks
// ---------------------------------------------------------------------------

void USimpleEnemyAI::TickIdle(float DeltaTime)
{
	// Try to find the player
	if (!TargetPlayer.IsValid())
	{
		TargetPlayer = FindPlayer();
	}

	// Patrol while idle
	if (bEnablePatrol && (!TargetPlayer.IsValid() || GetDistanceToTarget() > DetectionRange))
	{
		ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
		if (OwnerChar)
		{
			if (bIsWaiting)
			{
				PatrolWaitTimer -= DeltaTime;
				if (PatrolWaitTimer <= 0.0f)
				{
					bIsWaiting = false;
					PickNewPatrolTarget();
				}
			}
			else
			{
				// Move towards patrol target
				const float DistToTarget = FVector::Dist2D(OwnerChar->GetActorLocation(), PatrolTarget);
				if (DistToTarget < 50.0f)
				{
					bIsWaiting = true;
					PatrolWaitTimer = PatrolWaitTime;
				}
				else
				{
					const FVector Dir = (PatrolTarget - OwnerChar->GetActorLocation()).GetSafeNormal();
					OwnerChar->AddMovementInput(Dir, 1.0f);
					if (OwnerChar->GetCharacterMovement())
					{
						OwnerChar->GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
					}

					// Face movement direction
					const FRotator DesiredRot = Dir.Rotation();
					OwnerChar->SetActorRotation(FMath::RInterpTo(
						OwnerChar->GetActorRotation(), FRotator(0, DesiredRot.Yaw, 0), DeltaTime, 4.0f));
				}
			}
		}
	}

	if (TargetPlayer.IsValid() && GetDistanceToTarget() <= DetectionRange)
	{
		AIState = ESimpleAIState::Chase;

		if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetOwner()))
		{
			Enemy->SetAwarenessState(EEnemyAwareness::Combat);

			// Alert nearby enemies
			TArray<AActor*> NearbyEnemies;
			UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), NearbyEnemies);
			for (AActor* Other : NearbyEnemies)
			{
				if (Other == GetOwner()) continue;
				if (FVector::Dist(GetOwner()->GetActorLocation(), Other->GetActorLocation()) > 1200.0f) continue;

				if (USimpleEnemyAI* OtherAI = Other->FindComponentByClass<USimpleEnemyAI>())
				{
					OtherAI->AlertToThreat(TargetPlayer.Get());
				}
			}
		}
	}
}

void USimpleEnemyAI::TickChase(float DeltaTime)
{
	if (!TargetPlayer.IsValid())
	{
		AIState = ESimpleAIState::Idle;
		return;
	}

	const float Distance = GetDistanceToTarget();

	// Lost target
	if (Distance > DetectionRange * 1.5f)
	{
		AIState = ESimpleAIState::Idle;
		if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetOwner()))
		{
			Enemy->SetAwarenessState(EEnemyAwareness::Suspicious);
		}
		return;
	}

	// In attack range
	if (Distance <= AttackRange)
	{
		AIState = ESimpleAIState::Attack;
		return;
	}

	FaceTarget(DeltaTime);
	MoveTowardsTarget(DeltaTime);
}

void USimpleEnemyAI::TickAttack(float DeltaTime)
{
	if (!TargetPlayer.IsValid())
	{
		AIState = ESimpleAIState::Idle;
		return;
	}

	const float Distance = GetDistanceToTarget();

	// Target moved out of range — chase
	if (Distance > AttackRange * 1.3f)
	{
		AIState = ESimpleAIState::Chase;
		return;
	}

	FaceTarget(DeltaTime);

	// Attack on cooldown
	if (AttackCooldownRemaining <= 0.0f)
	{
		PerformAttack();
		AttackCooldownRemaining = AttackCooldown;
	}
}

void USimpleEnemyAI::TickStagger(float DeltaTime)
{
	StaggerTimer -= DeltaTime;
	if (StaggerTimer <= 0.0f)
	{
		AIState = TargetPlayer.IsValid() ? ESimpleAIState::Chase : ESimpleAIState::Idle;
	}
}

// ---------------------------------------------------------------------------
// Actions
// ---------------------------------------------------------------------------

void USimpleEnemyAI::PickNewPatrolTarget()
{
	const float Angle = FMath::FRandRange(0.0f, 360.0f);
	const float Dist = FMath::FRandRange(PatrolRadius * 0.3f, PatrolRadius);
	PatrolTarget = SpawnLocation + FVector(
		FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
		FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
		0.0f);
}

void USimpleEnemyAI::AlertToThreat(AActor* Threat)
{
	if (!Threat || AIState == ESimpleAIState::Dead || AIState == ESimpleAIState::Chase ||
		AIState == ESimpleAIState::Attack)
	{
		return;
	}

	TargetPlayer = Threat;
	AIState = ESimpleAIState::Chase;

	if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetOwner()))
	{
		Enemy->SetAwarenessState(EEnemyAwareness::Alert);
	}

	UE_LOG(LogAnansi, Verbose, TEXT("%s alerted to threat %s"), *GetOwner()->GetName(), *Threat->GetName());
}

AActor* USimpleEnemyAI::FindPlayer() const
{
	return UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

float USimpleEnemyAI::GetDistanceToTarget() const
{
	if (!TargetPlayer.IsValid() || !GetOwner())
	{
		return MAX_FLT;
	}
	return FVector::Dist(GetOwner()->GetActorLocation(), TargetPlayer->GetActorLocation());
}

void USimpleEnemyAI::FaceTarget(float DeltaTime)
{
	if (!TargetPlayer.IsValid() || !GetOwner())
	{
		return;
	}

	const FVector Direction = (TargetPlayer->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
	const FRotator DesiredRot = Direction.Rotation();
	const FRotator CurrentRot = GetOwner()->GetActorRotation();
	const FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, 8.0f);

	GetOwner()->SetActorRotation(FRotator(0, NewRot.Yaw, 0));
}

void USimpleEnemyAI::MoveTowardsTarget(float DeltaTime)
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar || !TargetPlayer.IsValid())
	{
		return;
	}

	const FVector Direction = (TargetPlayer->GetActorLocation() - OwnerChar->GetActorLocation()).GetSafeNormal();
	OwnerChar->AddMovementInput(Direction, 1.0f);

	if (UCharacterMovementComponent* CMC = OwnerChar->GetCharacterMovement())
	{
		CMC->MaxWalkSpeed = ChaseSpeed;
	}
}

void USimpleEnemyAI::PerformAttack()
{
	AActor* Owner = GetOwner();
	AActor* Target = TargetPlayer.Get();
	if (!Owner || !Target)
	{
		return;
	}

	// Apply damage to the player
	FDamageEvent DamageEvent;
	Target->TakeDamage(AttackDamage, DamageEvent, nullptr, Owner);

	// Show damage number on the HUD
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->AddDamageNumber(AttackDamage, Target->GetActorLocation() + FVector(0, 0, 80), false);
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("%s attacks player for %.0f damage"), *Owner->GetName(), AttackDamage);
}

void USimpleEnemyAI::OnOwnerTakeDamage(AActor* DamagedActor, float Damage,
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (AIState == ESimpleAIState::Dead)
	{
		return;
	}

	// Show damage number
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->AddDamageNumber(Damage, DamagedActor->GetActorLocation() + FVector(0, 0, 100), Damage >= 25.0f);
		}
	}

	// Stagger on hit
	if (Damage >= 10.0f)
	{
		AIState = ESimpleAIState::Stagger;
		StaggerTimer = StaggerDuration;
	}

	// Acquire target if we don't have one
	if (!TargetPlayer.IsValid() && DamageCauser)
	{
		TargetPlayer = DamageCauser;
	}
}
