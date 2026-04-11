// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimpleEnemyAI.generated.h"

class AEnemyBase;
class UMeleeDamageDealer;

/**
 * USimpleEnemyAI
 *
 * Tick-based enemy AI component that provides basic behaviors without
 * requiring BehaviorTrees, Blackboards, or NavMesh. Suitable for
 * development testing and vertical slice prototyping.
 *
 * Behaviors:
 *  - IDLE: Stand still, detect player in range
 *  - CHASE: Move towards player using simple steering
 *  - ATTACK: When in melee range, deal damage on a cooldown
 *  - STAGGER: Briefly stunned after being hit
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API USimpleEnemyAI : public UActorComponent
{
	GENERATED_BODY()

public:
	USimpleEnemyAI();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// -- Configuration -------------------------------------------------------

	/** Detection range for spotting the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|AI", meta = (ClampMin = "100.0"))
	float DetectionRange = 1500.0f;

	/** Range at which the enemy starts attacking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|AI", meta = (ClampMin = "50.0"))
	float AttackRange = 200.0f;

	/** Cooldown between attacks in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|AI", meta = (ClampMin = "0.1"))
	float AttackCooldown = 2.0f;

	/** Damage per attack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|AI", meta = (ClampMin = "1.0"))
	float AttackDamage = 10.0f;

	/** Movement speed when chasing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|AI", meta = (ClampMin = "50.0"))
	float ChaseSpeed = 400.0f;

	/** How long the enemy is staggered after a big hit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|AI", meta = (ClampMin = "0.0"))
	float StaggerDuration = 0.8f;

	/** Enable simple patrol behavior when idle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|AI")
	bool bEnablePatrol = true;

	/** Patrol radius from spawn point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|AI", meta = (ClampMin = "100.0"))
	float PatrolRadius = 400.0f;

	/** Speed while patrolling (slower than chase). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|AI", meta = (ClampMin = "50.0"))
	float PatrolSpeed = 150.0f;

	/** Seconds to wait at each patrol point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|AI", meta = (ClampMin = "0.0"))
	float PatrolWaitTime = 2.0f;

	/** Alert this AI about a threat. Used for alert propagation. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|AI")
	void AlertToThreat(AActor* Threat);

// Internal — exposed to allow alert propagation between AI components
// (friend not needed since AlertToThreat is the public API)
private:
	enum class ESimpleAIState : uint8
	{
		Idle,
		Chase,
		Attack,
		Stagger,
		Dead
	};

	ESimpleAIState AIState = ESimpleAIState::Idle;

	UPROPERTY()
	TWeakObjectPtr<AActor> TargetPlayer;

	float AttackCooldownRemaining = 0.0f;
	float StaggerTimer = 0.0f;

	FVector SpawnLocation = FVector::ZeroVector;
	FVector PatrolTarget = FVector::ZeroVector;
	float PatrolWaitTimer = 0.0f;
	bool bIsWaiting = false;

	void TickIdle(float DeltaTime);
	void TickChase(float DeltaTime);
	void TickAttack(float DeltaTime);
	void TickStagger(float DeltaTime);

	AActor* FindPlayer() const;
	float GetDistanceToTarget() const;
	void FaceTarget(float DeltaTime);
	void MoveTowardsTarget(float DeltaTime);
	void PerformAttack();
	void PickNewPatrolTarget();

	UFUNCTION()
	void OnOwnerTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);
};
