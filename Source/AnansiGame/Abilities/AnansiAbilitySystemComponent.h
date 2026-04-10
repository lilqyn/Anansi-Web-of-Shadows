// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AnansiAbilitySystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebEnergyChanged, float, NewEnergy, float, MaxEnergy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityActivationFailed, const UGameplayAbility*, Ability);

/**
 * UAnansiAbilitySystemComponent
 *
 * Extended Ability System Component for Anansi. Manages web-energy as a custom
 * resource pool, tracks active abilities, and provides cooldown helpers.
 */
UCLASS(ClassGroup = (Anansi), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UAnansiAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UAnansiAbilitySystemComponent();

	// -------------------------------------------------------------------
	// UActorComponent interface
	// -------------------------------------------------------------------

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// -------------------------------------------------------------------
	// Web energy management
	// -------------------------------------------------------------------

	/** Attempt to spend energy. Returns true if the character had enough. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Abilities|Energy")
	bool SpendWebEnergy(float Amount);

	/** Add energy (e.g. from pickups). Clamped to max. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Abilities|Energy")
	void AddWebEnergy(float Amount);

	/** Set the maximum energy pool. Current energy is clamped accordingly. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Abilities|Energy")
	void SetMaxWebEnergy(float NewMax);

	UFUNCTION(BlueprintPure, Category = "Anansi|Abilities|Energy")
	float GetCurrentWebEnergy() const { return CurrentWebEnergy; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Abilities|Energy")
	float GetMaxWebEnergy() const { return MaxWebEnergy; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Abilities|Energy")
	float GetWebEnergyPercent() const;

	// -------------------------------------------------------------------
	// Cooldown helpers
	// -------------------------------------------------------------------

	/** Returns remaining cooldown for the given ability class, or 0 if ready. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Abilities|Cooldown")
	float GetCooldownRemainingForAbility(TSubclassOf<UGameplayAbility> AbilityClass) const;

	/** Returns true when the ability class has no active cooldown. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Abilities|Cooldown")
	bool IsAbilityOffCooldown(TSubclassOf<UGameplayAbility> AbilityClass) const;

	// -------------------------------------------------------------------
	// Activation overrides
	// -------------------------------------------------------------------

	/** Custom pre-activation checks (energy, stun, etc.). */
	virtual bool TryActivateAbilityWithTag(FGameplayTag AbilityTag);

	/** Track ability activation and deactivation. */
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;

	// -------------------------------------------------------------------
	// Active ability tracking
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Abilities")
	int32 GetActiveAbilityCount() const { return ActiveAbilityCount; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Abilities")
	bool HasActiveAbilityWithTag(FGameplayTag Tag) const;

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Abilities|Energy")
	FOnWebEnergyChanged OnWebEnergyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Abilities")
	FOnAbilityActivationFailed OnAbilityActivationFailed;

protected:
	/** Maximum web energy. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Abilities|Energy", meta = (ClampMin = "1.0"))
	float MaxWebEnergy = 100.0f;

	/** Energy regenerated per second while not spending. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Abilities|Energy", meta = (ClampMin = "0.0"))
	float WebEnergyRegenRate = 5.0f;

	/** Delay after spending energy before regen resumes (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Abilities|Energy", meta = (ClampMin = "0.0"))
	float RegenCooldownDuration = 2.0f;

private:
	float CurrentWebEnergy = 0.0f;
	float RegenCooldownRemaining = 0.0f;
	int32 ActiveAbilityCount = 0;

	void BroadcastEnergyChange();
};
