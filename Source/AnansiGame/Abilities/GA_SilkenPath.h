// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AnansiGameplayAbility.h"
#include "GA_SilkenPath.generated.h"

class ANiagaraActor;
class UNiagaraSystem;

/**
 * UGA_SilkenPath
 *
 * Shoots a web line forward. If it strikes world geometry a temporary web bridge
 * is spawned for traversal. If it strikes an enemy, the enemy is pulled toward
 * the player. A Niagara web-thread VFX is drawn along the path.
 */
UCLASS()
class ANANSIGAME_API UGA_SilkenPath : public UAnansiGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SilkenPath();

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

	/** Maximum distance the web line can travel. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SilkenPath")
	float WebTraceDistance = 3000.0f;

	/** How long the web bridge remains before dissolving (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SilkenPath", meta = (ClampMin = "1.0"))
	float BridgeDuration = 8.0f;

	/** Pull speed applied to enemies hit by the web. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SilkenPath", meta = (ClampMin = "0.0"))
	float EnemyPullSpeed = 2000.0f;

	/** Blueprint class for the web bridge actor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SilkenPath")
	TSubclassOf<AActor> WebBridgeClass;

	/** Niagara system for the web thread visual effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|SilkenPath")
	TObjectPtr<UNiagaraSystem> WebThreadVFX;

private:
	/** Handle spawning a web bridge at the hit location. */
	void SpawnWebBridge(const FVector& Start, const FVector& End, UWorld* World);

	/** Handle pulling an enemy character toward the player. */
	void PullEnemy(AActor* Enemy, const FVector& PlayerLocation);

	/** Spawn the Niagara web thread effect between two points. */
	void SpawnWebThreadEffect(const FVector& Start, const FVector& End, UWorld* World);

	UPROPERTY()
	TObjectPtr<AActor> SpawnedBridge;

	FTimerHandle BridgeTimerHandle;
};
