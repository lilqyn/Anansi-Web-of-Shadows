// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class UEnemyCombatComponent;

/**
 * Enemy archetypes — each has distinct visual language, behavior, and stats.
 */
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Guard	UMETA(DisplayName = "Guard"),
	Zealot	UMETA(DisplayName = "Zealot"),
	Spirit	UMETA(DisplayName = "Spirit"),
	Beast	UMETA(DisplayName = "Beast"),
	Shade	UMETA(DisplayName = "Shade")
};

/**
 * Awareness states for stealth integration.
 */
UENUM(BlueprintType)
enum class EEnemyAwareness : uint8
{
	Unaware		UMETA(DisplayName = "Unaware"),
	Suspicious	UMETA(DisplayName = "Suspicious"),
	Alert		UMETA(DisplayName = "Alert"),
	Combat		UMETA(DisplayName = "Combat")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDied, AEnemyBase*, Enemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyAwarenessChanged, EEnemyAwareness, OldState, EEnemyAwareness, NewState);

/**
 * AEnemyBase
 *
 * Base character for all enemy types. Provides health, combat stats, death
 * handling, damage response (hit reactions, stagger, knockback), and stealth
 * awareness tracking.
 */
UCLASS()
class ANANSIGAME_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	virtual void BeginPlay() override;

	// -------------------------------------------------------------------
	// Damage interface
	// -------------------------------------------------------------------

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Anansi|Enemy")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Enemy")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Enemy")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Enemy")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	// -------------------------------------------------------------------
	// Enemy identity
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Enemy")
	EEnemyType GetEnemyType() const { return EnemyType; }

	// -------------------------------------------------------------------
	// Awareness (stealth)
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Enemy")
	EEnemyAwareness GetAwarenessState() const { return AwarenessState; }

	UFUNCTION(BlueprintCallable, Category = "Anansi|Enemy")
	void SetAwarenessState(EEnemyAwareness NewState);

	// -------------------------------------------------------------------
	// Damage responses
	// -------------------------------------------------------------------

	/** Trigger a hit reaction montage. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Enemy")
	void PlayHitReaction(const FVector& HitDirection);

	/** Trigger a stagger state for a short duration. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Enemy")
	void ApplyStagger(float Duration);

	/** Apply a knockback impulse. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Enemy")
	void ApplyKnockback(const FVector& Direction, float Force);

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Enemy")
	FOnEnemyHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Enemy")
	FOnEnemyDied OnDied;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Enemy")
	FOnEnemyAwarenessChanged OnAwarenessChanged;

protected:
	// -------------------------------------------------------------------
	// Stats
	// -------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Stats")
	EEnemyType EnemyType = EEnemyType::Guard;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Stats", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Stats", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Stats", meta = (ClampMin = "0.1"))
	float AttackSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Stats", meta = (ClampMin = "0.0"))
	float AttackRange = 200.0f;

	/** Damage threshold to trigger a stagger. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Stats", meta = (ClampMin = "0.0"))
	float StaggerThreshold = 25.0f;

	// -------------------------------------------------------------------
	// Death configuration
	// -------------------------------------------------------------------

	/** Enable ragdoll on death. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Death")
	bool bRagdollOnDeath = true;

	/** Loot table / loot drop class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Death")
	TSubclassOf<AActor> LootDropClass;

	/** Niagara death VFX. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Death")
	TObjectPtr<class UNiagaraSystem> DeathVFX;

	/** Death sound. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Death")
	TObjectPtr<USoundBase> DeathSound;

	// -------------------------------------------------------------------
	// Animation montages
	// -------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Animation")
	TObjectPtr<UAnimMontage> HitReactionMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Enemy|Animation")
	TObjectPtr<UAnimMontage> StaggerMontage;

private:
	void HandleDeath(AController* Killer);
	void SpawnLoot();
	void EnableRagdoll();

	float CurrentHealth = 0.0f;
	bool bIsDead = false;
	float AccumulatedDamage = 0.0f;
	EEnemyAwareness AwarenessState = EEnemyAwareness::Unaware;

	FTimerHandle StaggerTimerHandle;
};
