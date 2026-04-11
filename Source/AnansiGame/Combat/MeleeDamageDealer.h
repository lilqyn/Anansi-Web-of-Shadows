// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeleeDamageDealer.generated.h"

/**
 * UMeleeDamageDealer
 *
 * Simple proximity-based melee damage component that works without
 * skeletal mesh sockets or animation montages. When FireAttack() is called,
 * it sphere-traces in front of the owner and applies UE damage to all
 * hit actors.
 *
 * Used as a fallback when no weapon mesh/sockets exist (e.g., meshless testing).
 * In production, this is replaced by the WeaponTrace + AnimNotify pipeline.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UMeleeDamageDealer : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeleeDamageDealer();

	/**
	 * Fire a melee attack — sphere trace forward and apply damage to hits.
	 * Returns the number of actors hit.
	 */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Combat")
	int32 FireAttack(float DamageOverride = -1.0f);

	/** Damage dealt per hit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat", meta = (ClampMin = "0.0"))
	float BaseDamage = 15.0f;

	/** Range of the melee attack trace. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat", meta = (ClampMin = "50.0"))
	float AttackRange = 200.0f;

	/** Radius of the attack sphere trace. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat", meta = (ClampMin = "10.0"))
	float AttackRadius = 60.0f;

	/** Knockback force applied on hit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat", meta = (ClampMin = "0.0"))
	float KnockbackForce = 400.0f;

	/** Whether to show debug traces. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat")
	bool bDrawDebug = false;
};
