// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Player/AnansiCharacter.h"
#include "AnansiAnimInstance.generated.h"

/**
 * UAnansiAnimInstance
 *
 * Custom animation instance that exposes character state to the Animation
 * Blueprint via blueprint-readable properties. Updated every frame from the
 * owning AAnansiCharacter's authoritative state so that the anim graph can
 * blend locomotion, combat, stealth, and traversal layers.
 *
 * Design note: All properties are updated in NativeUpdateAnimation (C++ side)
 * for performance. The Animation Blueprint reads these values in its state
 * machine transitions and blend nodes — no Tick is needed on the BP side.
 */
UCLASS()
class ANANSIGAME_API UAnansiAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UAnansiAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// -----------------------------------------------------------------------
	// Locomotion
	// -----------------------------------------------------------------------

	/** Ground speed in cm/s (used for walk/run blend). */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Locomotion")
	float MovementSpeed = 0.0f;

	/** Movement direction relative to the character's facing (degrees, -180 to 180). */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Locomotion")
	float MovementDirection = 0.0f;

	/** True when the character is in the air (jumping or falling). */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Locomotion")
	bool bIsJumping = false;

	/** True when the character is falling (post-apex of a jump or walked off a ledge). */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Locomotion")
	bool bIsFalling = false;

	/** True when the character is running along a wall surface. */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Locomotion")
	bool bIsWallRunning = false;

	/** True when the character is mid web-swing. */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Locomotion")
	bool bIsWebSwinging = false;

	// -----------------------------------------------------------------------
	// Combat
	// -----------------------------------------------------------------------

	/** True when any attack montage is playing. */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Combat")
	bool bIsAttacking = false;

	/** Current index in the active combo chain (0-based). */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Combat")
	int32 ComboIndex = 0;

	/** True during the parry anticipation window. */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Combat")
	bool bIsParrying = false;

	/** True during the dodge/roll animation. */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Combat")
	bool bIsDodging = false;

	/** True when the character is locked on to a target. */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Combat")
	bool bIsLockedOn = false;

	// -----------------------------------------------------------------------
	// Stealth
	// -----------------------------------------------------------------------

	/** True when the character is crouching / in stealth mode. */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Stealth")
	bool bIsCrouching = false;

	/** Opacity value for stealth material blending (0 = invisible, 1 = fully visible). */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Stealth")
	float StealthOpacity = 1.0f;

	// -----------------------------------------------------------------------
	// Ability state
	// -----------------------------------------------------------------------

	/** True when the character is performing a GAS ability with a montage. */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|Ability")
	bool bIsUsingAbility = false;

	// -----------------------------------------------------------------------
	// General state
	// -----------------------------------------------------------------------

	/** Current high-level character state enum — useful for state-machine transitions. */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|State")
	EAnansiCharacterState CharacterState = EAnansiCharacterState::Idle;

	/** True when the character is alive. */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|State")
	bool bIsAlive = true;

	/** Normalised health (0-1) for HUD-tied blend effects (e.g., wounded idle). */
	UPROPERTY(BlueprintReadOnly, Category = "Anansi|Anim|State")
	float HealthPercent = 1.0f;

private:
	/** Cached reference to the owning Anansi character. Set once in NativeInitializeAnimation. */
	UPROPERTY()
	TWeakObjectPtr<AAnansiCharacter> OwnerCharacter;
};
