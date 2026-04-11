// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Stealth/StealthComponent.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/EnemyBase.h"
#include "Engine/World.h"
#include "Components/LightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

UStealthComponent::UStealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz is sufficient
}

void UStealthComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateLightLevel();
	UpdateNoiseLevel();
	UpdateVisibilityScore();
	UpdateStealthState();
}

// ---------------------------------------------------------------------------
// Light level
// ---------------------------------------------------------------------------

void UStealthComponent::UpdateLightLevel()
{
	// Approximate light level by tracing upward and checking for sky exposure,
	// combined with a scene luminance query at the player location.
	// A production implementation would use a render-target readback or the
	// exposure system; here we use a simplified line-trace heuristic.

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector Location = Owner->GetActorLocation();
	UWorld* World = GetWorld();

	// Trace upward — if blocked, we're under cover (lower light)
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	const bool bUnderCover = World->LineTraceSingleByChannel(
		Hit, Location, Location + FVector(0.0f, 0.0f, 2000.0f), ECC_Visibility, Params);

	LightLevel = bUnderCover ? 0.3f : 0.9f;

	// Time-of-day adjustment (simple)
	if (const APlayerController* PC = Cast<APlayerController>(
		Cast<ACharacter>(Owner)->GetController()))
	{
		// Could integrate with a day-night system here
	}
}

// ---------------------------------------------------------------------------
// Noise level
// ---------------------------------------------------------------------------

void UStealthComponent::UpdateNoiseLevel()
{
	const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		NoiseLevel = 0.0f;
		return;
	}

	const UCharacterMovementComponent* Movement = OwnerChar->GetCharacterMovement();
	if (!Movement)
	{
		NoiseLevel = 0.0f;
		return;
	}

	const float Speed = Movement->Velocity.Size();

	if (Speed < 1.0f)
	{
		NoiseLevel = 0.0f;
	}
	else if (OwnerChar->bIsCrouched)
	{
		NoiseLevel = CrouchNoiseLevel;
	}
	else if (Movement->MaxWalkSpeed > 0.0f && Speed > Movement->MaxWalkSpeed * 0.8f)
	{
		NoiseLevel = SprintNoiseLevel;
	}
	else
	{
		NoiseLevel = WalkNoiseLevel;
	}
}

// ---------------------------------------------------------------------------
// Visibility
// ---------------------------------------------------------------------------

void UStealthComponent::UpdateVisibilityScore()
{
	// Combine factors: light, noise, cover
	const float LightFactor = LightLevel;
	const float NoiseFactor = NoiseLevel;
	const float CoverFactor = FMath::Clamp(1.0f - CoverBonusValue, 0.0f, 1.0f);

	// Weighted combination
	VisibilityScore = FMath::Clamp(
		LightFactor * 0.4f + NoiseFactor * 0.3f + CoverFactor * 0.3f,
		0.0f, 1.0f);
}

void UStealthComponent::UpdateStealthState()
{
	EStealthState NewState;

	if (VisibilityScore <= HiddenThreshold)
	{
		NewState = EStealthState::Hidden;
	}
	else if (VisibilityScore <= PartiallyHiddenThreshold)
	{
		NewState = EStealthState::PartiallyHidden;
	}
	else
	{
		NewState = EStealthState::Visible;
	}

	if (NewState != CurrentState)
	{
		const EStealthState OldState = CurrentState;
		CurrentState = NewState;
		OnStealthStateChanged.Broadcast(OldState, NewState);

		if (NewState == EStealthState::Hidden)
		{
			OnEnteredStealth.Broadcast();
		}
		else if (OldState == EStealthState::Hidden && NewState == EStealthState::Visible)
		{
			OnDetected.Broadcast();
		}
	}
}

// ---------------------------------------------------------------------------
// Stealth takedown
// ---------------------------------------------------------------------------

bool UStealthComponent::CanPerformStealthTakedown(AActor* Enemy) const
{
	if (!Enemy || !GetOwner())
	{
		return false;
	}

	// Must be hidden or partially hidden
	if (CurrentState == EStealthState::Visible)
	{
		return false;
	}

	// Range check
	const float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Enemy->GetActorLocation());
	if (Distance > TakedownRange)
	{
		return false;
	}

	// Behind-the-enemy check: dot product of enemy forward vs direction-to-player
	const FVector EnemyForward = Enemy->GetActorForwardVector();
	const FVector ToPlayer = (GetOwner()->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
	const float DotProduct = FVector::DotProduct(EnemyForward, ToPlayer);

	// DotProduct < 0 means player is behind the enemy
	const float AngleCos = FMath::Cos(FMath::DegreesToRadians(TakedownAngleThreshold));
	if (DotProduct > -AngleCos)
	{
		return false;
	}

	// Enemy must be unaware
	if (const AEnemyBase* EnemyChar = Cast<AEnemyBase>(Enemy))
	{
		if (EnemyChar->GetAwarenessState() == EEnemyAwareness::Combat)
		{
			return false;
		}
	}

	return true;
}

void UStealthComponent::PerformStealthTakedown(AActor* Enemy)
{
	if (!CanPerformStealthTakedown(Enemy))
	{
		return;
	}

	// Apply lethal damage to the enemy
	if (AEnemyBase* EnemyChar = Cast<AEnemyBase>(Enemy))
	{
		const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
		AController* InstigatorController = OwnerChar ? OwnerChar->GetController() : nullptr;

		FDamageEvent DamageEvent;
		EnemyChar->TakeDamage(EnemyChar->GetMaxHealth() * 10.0f, DamageEvent,
			InstigatorController, GetOwner());
	}

	OnStealthKill.Broadcast(Enemy);

	UE_LOG(LogAnansi, Log, TEXT("Stealth takedown on %s"), *Enemy->GetName());
}

// ---------------------------------------------------------------------------
// Cover
// ---------------------------------------------------------------------------

void UStealthComponent::SetCoverBonus(float Bonus)
{
	CoverBonusValue = FMath::Clamp(Bonus, 0.0f, 1.0f);
}

void UStealthComponent::ClearCoverBonus()
{
	CoverBonusValue = 0.0f;
}
