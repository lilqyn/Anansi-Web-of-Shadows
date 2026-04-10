// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DamageTypes.generated.h"

/**
 * Categories of damage in the Anansi combat system.
 * Each type can have different resistance/weakness modifiers on enemies,
 * and triggers distinct VFX/SFX on hit.
 */
UENUM(BlueprintType)
enum class EAnansiDamageType : uint8
{
	/** Standard melee / physical impact. */
	Physical	UMETA(DisplayName = "Physical"),

	/** Spirit-realm damage — effective against shadow creatures. */
	Spirit		UMETA(DisplayName = "Spirit"),

	/** Fire damage — applies burn DOT. */
	Fire		UMETA(DisplayName = "Fire"),

	/** Poison damage — slows and damages over time. */
	Poison		UMETA(DisplayName = "Poison")
};

/**
 * Full damage payload passed between the combat system, weapon traces,
 * and health components. Carries everything the receiver needs to
 * calculate final damage and play reactions.
 */
USTRUCT(BlueprintType)
struct FAnansiDamageInfo
{
	GENERATED_BODY()

	/** Base damage amount before resistances. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float Amount = 0.0f;

	/** Damage category for resistance lookup. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	EAnansiDamageType DamageType = EAnansiDamageType::Physical;

	/** World-space direction the target should be knocked back. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FVector KnockbackDirection = FVector::ZeroVector;

	/** Magnitude of the knockback impulse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float KnockbackForce = 0.0f;

	/** World-space hit location for VFX spawning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FVector HitLocation = FVector::ZeroVector;

	/** Surface normal at the hit point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FVector HitNormal = FVector::UpVector;

	/** The actor that dealt this damage (can be nullptr for environmental). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TWeakObjectPtr<AActor> Attacker;

	/** Whether this hit was a critical / weak-point hit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bIsCritical = false;

	/** If true, the target cannot block/parry this damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bUnblockable = false;
};
