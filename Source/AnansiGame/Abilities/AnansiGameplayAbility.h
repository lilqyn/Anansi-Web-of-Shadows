// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AnansiGameplayAbility.generated.h"

class UAnansiAbilitySystemComponent;

/**
 * UAnansiGameplayAbility
 *
 * Base gameplay ability for every Anansi ability.  Adds web-energy cost,
 * cooldown duration, and activation guard-rails (stunned, dead, etc.).
 */
UCLASS(Abstract)
class ANANSIGAME_API UAnansiGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAnansiGameplayAbility();

	// -------------------------------------------------------------------
	// Cost / cooldown configuration
	// -------------------------------------------------------------------

	/** Web energy cost to activate this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Ability", meta = (ClampMin = "0.0"))
	float EnergyCost = 0.0f;

	/** Cooldown duration in seconds. Applied through a dynamic gameplay effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Ability", meta = (ClampMin = "0.0"))
	float CooldownDuration = 0.0f;

	/** Tags that block this ability from activating (e.g. State.Stunned). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Ability")
	FGameplayTagContainer BlockingTags;

	// -------------------------------------------------------------------
	// UGameplayAbility overrides
	// -------------------------------------------------------------------

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	// -------------------------------------------------------------------
	// Helpers — available to all subclasses
	// -------------------------------------------------------------------

	/** Get the owning Anansi ASC. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Ability")
	UAnansiAbilitySystemComponent* GetAnansiAbilitySystemComponent() const;

	/** Get the owning player character. Returns nullptr if the owner is not a character. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Ability")
	ACharacter* GetOwnerCharacter() const;

	/** Convenience — forward-facing line trace from the player camera. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Ability")
	bool PerformCameraLineTrace(float TraceDistance, FHitResult& OutHit) const;

protected:
	/** Called after energy is spent and the ability is officially active. Override in child classes. */
	virtual void OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData);

	/** Apply the cooldown using a transient gameplay effect. */
	void ApplyCooldownEffect();
};
