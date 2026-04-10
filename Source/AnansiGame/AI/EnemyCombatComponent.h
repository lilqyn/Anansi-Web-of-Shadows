// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyCombatComponent.generated.h"

/**
 * Describes a single attack pattern available to an enemy.
 */
USTRUCT(BlueprintType)
struct FEnemyAttackPattern
{
	GENERATED_BODY()

	/** Unique name for this attack. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName AttackName;

	/** Animation montage to play. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage;

	/** Damage dealt by this attack. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Damage = 10.0f;

	/** Minimum range to use this attack. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float MinRange = 0.0f;

	/** Maximum range to use this attack. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float MaxRange = 200.0f;

	/** Cooldown between uses of this specific attack (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Cooldown = 2.0f;

	/** Telegraph duration before the attack connects — gives the player reaction time. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float TelegraphDuration = 0.5f;

	/** Weight for selection randomization (higher = more likely). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.1"))
	float SelectionWeight = 1.0f;

	/** If true, this attack is only used when the player is airborne. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRequiresAirborneTarget = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyAttackStarted, const FEnemyAttackPattern&, Attack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyAttackEnded);

/**
 * UEnemyCombatComponent
 *
 * Manages enemy combat behavior: attack pattern selection, telegraph timing,
 * group coordination (prevent dog-piling), and block/dodge chance for
 * tougher enemies.
 */
UCLASS(ClassGroup = (Anansi), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UEnemyCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// -------------------------------------------------------------------
	// Attack interface
	// -------------------------------------------------------------------

	/** Select and execute the best attack for the current range and state. Returns true if an attack started. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|EnemyCombat")
	bool TryAttack(AActor* Target);

	/** Is an attack currently in progress? */
	UFUNCTION(BlueprintPure, Category = "Anansi|EnemyCombat")
	bool IsAttacking() const { return bIsAttacking; }

	/** Interrupt the current attack. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|EnemyCombat")
	void InterruptAttack();

	// -------------------------------------------------------------------
	// Group coordination
	// -------------------------------------------------------------------

	/** Check if this enemy is allowed to attack (group attack slot available). */
	UFUNCTION(BlueprintPure, Category = "Anansi|EnemyCombat")
	bool CanAttackInGroup() const;

	/** Request an attack slot from the group coordinator. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|EnemyCombat")
	bool RequestAttackSlot();

	/** Release the held attack slot. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|EnemyCombat")
	void ReleaseAttackSlot();

	// -------------------------------------------------------------------
	// Defensive
	// -------------------------------------------------------------------

	/** Roll a block check. Returns true if the enemy blocks the incoming attack. */
	UFUNCTION(BlueprintPure, Category = "Anansi|EnemyCombat")
	bool RollBlockChance() const;

	/** Roll a dodge check. */
	UFUNCTION(BlueprintPure, Category = "Anansi|EnemyCombat")
	bool RollDodgeChance() const;

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|EnemyCombat")
	FOnEnemyAttackStarted OnAttackStarted;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|EnemyCombat")
	FOnEnemyAttackEnded OnAttackEnded;

protected:
	// -------------------------------------------------------------------
	// Configuration
	// -------------------------------------------------------------------

	/** Available attack patterns. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|EnemyCombat")
	TArray<FEnemyAttackPattern> AttackPatterns;

	/** Global attack cooldown between any attacks (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|EnemyCombat", meta = (ClampMin = "0.0"))
	float GlobalAttackCooldown = 1.0f;

	/** Block chance (0-1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|EnemyCombat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BlockChance = 0.0f;

	/** Dodge chance (0-1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|EnemyCombat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DodgeChance = 0.0f;

	/** Max simultaneous attackers in this enemy's group. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|EnemyCombat", meta = (ClampMin = "1"))
	int32 MaxGroupAttackers = 2;

private:
	/** Select the best attack pattern for the given distance. */
	const FEnemyAttackPattern* SelectAttack(float DistanceToTarget, bool bTargetAirborne) const;

	/** Execute the selected attack pattern. */
	void ExecuteAttack(const FEnemyAttackPattern& Pattern, AActor* Target);

	/** Called when the attack montage finishes or is interrupted. */
	void OnAttackMontageEnded();

	bool bIsAttacking = false;
	bool bHasAttackSlot = false;
	float GlobalCooldownRemaining = 0.0f;

	/** Per-attack cooldown timers. */
	TMap<FName, float> AttackCooldowns;

	/** Shared group attack counter — simple approach using a static count per-world. */
	static int32 ActiveGroupAttackers;
};
