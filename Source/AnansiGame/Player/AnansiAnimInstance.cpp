// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Player/AnansiAnimInstance.h"
#include "Player/AnansiCharacter.h"
#include "Combat/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnansiAnimInstance::UAnansiAnimInstance()
{
}

void UAnansiAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<AAnansiCharacter>(TryGetPawnOwner());
}

void UAnansiAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter.IsValid())
	{
		// Retry in case the pawn was assigned after init.
		OwnerCharacter = Cast<AAnansiCharacter>(TryGetPawnOwner());
		if (!OwnerCharacter.IsValid())
		{
			return;
		}
	}

	const AAnansiCharacter* Character = OwnerCharacter.Get();
	const UCharacterMovementComponent* CMC = Character->GetCharacterMovement();

	// -- Locomotion ---------------------------------------------------------
	const FVector Velocity = CMC->Velocity;
	MovementSpeed = Velocity.Size2D();

	// Direction relative to the character's forward — used for strafe blending.
	if (MovementSpeed > 5.0f)
	{
		const FRotator ActorRotation = Character->GetActorRotation();
		const FRotator VelocityRotation = Velocity.ToOrientationRotator();
		const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(VelocityRotation, ActorRotation);
		MovementDirection = Delta.Yaw;
	}
	else
	{
		MovementDirection = 0.0f;
	}

	bIsJumping = CMC->IsFalling() && Velocity.Z > 0.0f;
	bIsFalling = CMC->IsFalling() && Velocity.Z <= 0.0f;
	bIsCrouching = CMC->IsCrouching();

	// -- High-level state ---------------------------------------------------
	CharacterState = Character->GetCharacterState();
	bIsAlive = Character->IsAlive();
	HealthPercent = (Character->GetMaxHealth() > 0.0f)
		? Character->GetCurrentHealth() / Character->GetMaxHealth()
		: 0.0f;

	bIsWallRunning  = (CharacterState == EAnansiCharacterState::WallRunning);
	bIsWebSwinging  = (CharacterState == EAnansiCharacterState::WebSwinging);
	bIsUsingAbility = (CharacterState == EAnansiCharacterState::UsingAbility);

	// -- Combat (delegated to CombatComponent) ------------------------------
	const UCombatComponent* Combat = Character->CombatComponent;
	if (Combat)
	{
		bIsAttacking = Combat->IsAttacking();
		bIsParrying  = Combat->IsParrying();
		bIsDodging   = (CharacterState == EAnansiCharacterState::Dodging);
		bIsLockedOn  = Combat->IsLockedOn();
		ComboIndex   = Combat->GetComboCount();
	}

	// -- Stealth ------------------------------------------------------------
	// StealthOpacity is driven by the game — in full stealth the character
	// material lerps towards transparency. For now, map crouch to a basic
	// opacity reduction.
	if (CharacterState == EAnansiCharacterState::Stealth || CharacterState == EAnansiCharacterState::Crouching)
	{
		StealthOpacity = FMath::FInterpTo(StealthOpacity, 0.4f, DeltaSeconds, 5.0f);
	}
	else
	{
		StealthOpacity = FMath::FInterpTo(StealthOpacity, 1.0f, DeltaSeconds, 5.0f);
	}
}
