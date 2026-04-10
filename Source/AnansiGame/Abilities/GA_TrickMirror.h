// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AnansiGameplayAbility.h"
#include "GA_TrickMirror.generated.h"

class UNiagaraSystem;

/**
 * UGA_TrickMirror
 *
 * Creates an illusory decoy at the player's position. Enemies aggro onto the
 * decoy, which mimics an idle animation. The decoy has limited health and
 * explodes on destruction dealing spirit damage. The player can re-activate
 * the ability to detonate the decoy early.
 */
UCLASS()
class ANANSIGAME_API UGA_TrickMirror : public UAnansiGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TrickMirror();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	// -------------------------------------------------------------------
	// Configuration
	// -------------------------------------------------------------------

	/** Blueprint class for the decoy actor. Must have a skeletal mesh and health interface. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|TrickMirror")
	TSubclassOf<AActor> DecoyClass;

	/** How long the decoy persists if not destroyed (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|TrickMirror", meta = (ClampMin = "1.0"))
	float DecoyLifespan = 10.0f;

	/** Radius of the spirit-damage explosion on decoy destruction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|TrickMirror", meta = (ClampMin = "0.0"))
	float ExplosionRadius = 400.0f;

	/** Spirit damage dealt by the explosion. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|TrickMirror", meta = (ClampMin = "0.0"))
	float ExplosionDamage = 50.0f;

	/** Niagara system for the explosion VFX. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|TrickMirror")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;

	/** Sound played on explosion. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|TrickMirror")
	TObjectPtr<USoundBase> ExplosionSound;

private:
	/** Spawn the decoy at the player's current location and rotation. */
	void SpawnDecoy();

	/** Detonate the decoy, dealing radial spirit damage. */
	void DetonateDecoy();

	/** Called when decoy lifespan expires. */
	void OnDecoyExpired();

	/** Returns true when a decoy is currently active. */
	bool IsDecoyActive() const { return SpawnedDecoy != nullptr; }

	UPROPERTY()
	TObjectPtr<AActor> SpawnedDecoy;

	FTimerHandle DecoyTimerHandle;

	/** If true, re-activation will detonate rather than spawn. */
	bool bDecoyIsActive = false;
};
