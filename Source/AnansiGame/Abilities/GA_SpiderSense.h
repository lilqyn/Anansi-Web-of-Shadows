// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AnansiGameplayAbility.h"
#include "GA_SpiderSense.generated.h"

class UPostProcessComponent;
class UMaterialInstanceDynamic;

/**
 * UGA_SpiderSense
 *
 * Activates a heightened perception state: brief slow-motion, a radial pulse
 * post-process wave, and highlights on enemies, interactive objects, hidden
 * collectibles, traps, and enemy weak points.
 */
UCLASS()
class ANANSIGAME_API UGA_SpiderSense : public UAnansiGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SpiderSense();

protected:
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

	/** Detection radius around the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SpiderSense")
	float DetectionRadius = 3000.0f;

	/** Duration of the spider-sense effect (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SpiderSense", meta = (ClampMin = "0.5"))
	float SenseDuration = 4.0f;

	/** Time dilation applied during the initial pulse. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SpiderSense", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float SlowMotionTimeDilation = 0.3f;

	/** How long the slow-motion lasts before returning to normal (seconds, real-time). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SpiderSense", meta = (ClampMin = "0.1"))
	float SlowMotionDuration = 0.8f;

	/** Post-process material for the pulse-wave effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SpiderSense")
	TObjectPtr<UMaterialInterface> PulsePostProcessMaterial;

	/** Tag identifying actors as interactive/highlightable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SpiderSense")
	FName InteractableTag = FName("Interactable");

	/** Tag identifying hidden collectibles. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SpiderSense")
	FName CollectibleTag = FName("Collectible");

	/** Tag identifying traps. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SpiderSense")
	FName TrapTag = FName("Trap");

private:
	/** Scan for and highlight actors in radius. */
	void PerformDetectionScan(const FVector& Origin, UWorld* World);

	/** Apply a render custom depth highlight to an actor. */
	void HighlightActor(AActor* Actor, int32 StencilValue);

	/** Remove highlights from all tracked actors. */
	void ClearHighlights();

	/** Restore time dilation to normal. */
	void RestoreTimeDilation();

	/** End the sense effect after duration. */
	void OnSenseExpired();

	UPROPERTY()
	TArray<TObjectPtr<AActor>> HighlightedActors;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PulseMID;

	FTimerHandle SlowMoTimerHandle;
	FTimerHandle SenseTimerHandle;
};
