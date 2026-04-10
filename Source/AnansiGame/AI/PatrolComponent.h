// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PatrolComponent.generated.h"

class UBlackboardComponent;
class USplineComponent;

/**
 * UPatrolComponent
 *
 * Drives enemy patrol behavior along a series of waypoints or a spline path.
 * Integrates with the behavior tree via blackboard keys for the current
 * patrol target and wait timers.
 */
UCLASS(ClassGroup = (Anansi), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UPatrolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPatrolComponent();

	virtual void BeginPlay() override;

	// -------------------------------------------------------------------
	// Patrol interface
	// -------------------------------------------------------------------

	/** Get the world location of the current patrol target. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Patrol")
	FVector GetCurrentPatrolPoint() const;

	/** Advance to the next patrol point. Wraps around. Returns new target location. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Patrol")
	FVector AdvanceToNextPoint();

	/** Get the wait time for the current patrol point. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Patrol")
	float GetCurrentWaitTime() const;

	/** Get total number of patrol points. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Patrol")
	int32 GetPatrolPointCount() const;

	/** Get the current patrol index. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Patrol")
	int32 GetCurrentPatrolIndex() const { return CurrentPointIndex; }

	/** Returns true when patrol points are configured. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Patrol")
	bool HasPatrolPath() const;

	/** Write the current patrol target to a blackboard key. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Patrol")
	void WritePatrolTargetToBlackboard(UBlackboardComponent* Blackboard, FName KeyName);

	// -------------------------------------------------------------------
	// Configuration
	// -------------------------------------------------------------------

	/** Waypoint-based patrol points (world locations). */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Anansi|Patrol", meta = (MakeEditWidget = true))
	TArray<FVector> PatrolPoints;

	/** Optional spline actor for spline-based patrol. If set, overrides waypoints. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Anansi|Patrol")
	TObjectPtr<AActor> SplineActor;

	/** Movement speed while patrolling. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Patrol", meta = (ClampMin = "0.0"))
	float PatrolSpeed = 200.0f;

	/** Default wait time at each point (seconds). Can be overridden per-point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Patrol", meta = (ClampMin = "0.0"))
	float DefaultWaitTime = 2.0f;

	/** Per-point wait times. If shorter than PatrolPoints, remaining use DefaultWaitTime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Patrol")
	TArray<float> WaitTimes;

	/** Maximum random deviation from the patrol path (units). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Patrol", meta = (ClampMin = "0.0"))
	float RandomDeviation = 50.0f;

	/** Acceptance radius to consider a patrol point reached. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Patrol", meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 50.0f;

private:
	/** Resolve patrol points from spline if a spline actor is set. */
	void ResolveSplinePoints();

	/** Apply random deviation to a point. */
	FVector ApplyDeviation(const FVector& Point) const;

	int32 CurrentPointIndex = 0;

	/** Cached spline points after resolution. */
	TArray<FVector> ResolvedPoints;
};
