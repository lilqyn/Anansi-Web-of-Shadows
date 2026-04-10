// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AnansiGameplayAbility.h"
#include "GA_StoryBind.generated.h"

class UNiagaraSystem;
class UStoryFragmentSystem;

/**
 * UGA_StoryBind
 *
 * Uses recovered tale fragments to restrain or influence supernatural beings.
 * Against spirit-type enemies it roots them with golden chains; against bosses
 * it creates a vulnerability window. The effect scales with the number of
 * story fragments collected.
 */
UCLASS()
class ANANSIGAME_API UGA_StoryBind : public UAnansiGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_StoryBind();

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

	/** Minimum story fragments required to use this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|StoryBind", meta = (ClampMin = "1"))
	int32 MinFragmentsRequired = 1;

	/** Maximum range for targeting an enemy with Story Bind. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|StoryBind")
	float TargetRange = 1500.0f;

	/** Base root duration for spirit enemies (seconds). Scales with fragment count. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|StoryBind", meta = (ClampMin = "1.0"))
	float BaseRootDuration = 3.0f;

	/** Additional root time per story fragment beyond the minimum. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|StoryBind", meta = (ClampMin = "0.0"))
	float DurationPerExtraFragment = 0.5f;

	/** Duration of the vulnerability window on bosses (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|StoryBind", meta = (ClampMin = "1.0"))
	float BossVulnerabilityDuration = 4.0f;

	/** Niagara system for the golden chains effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|StoryBind")
	TObjectPtr<UNiagaraSystem> GoldenChainsVFX;

	/** Sound played when binding a target. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|StoryBind")
	TObjectPtr<USoundBase> BindSound;

	/** Tag on actors that are spirit-type enemies. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|StoryBind")
	FName SpiritTag = FName("Spirit");

	/** Tag on actors that are bosses. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|StoryBind")
	FName BossTag = FName("Boss");

private:
	/** Get story fragment system from the game instance. */
	UStoryFragmentSystem* GetStoryFragmentSystem() const;

	/** Calculate total root duration based on fragment count. */
	float CalculateRootDuration(int32 FragmentCount) const;

	/** Apply the root/bind effect to the target. */
	void ApplyBindEffect(AActor* Target, float Duration);

	/** Release the bound target. */
	void ReleaseBind();

	UPROPERTY()
	TObjectPtr<AActor> BoundTarget;

	FTimerHandle BindTimerHandle;
};
