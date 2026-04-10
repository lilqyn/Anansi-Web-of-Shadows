// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AnansiAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;

/**
 * AI states used by the behavior tree and blackboard.
 */
UENUM(BlueprintType)
enum class EAnansiAIState : uint8
{
	Patrol		UMETA(DisplayName = "Patrol"),
	Investigate	UMETA(DisplayName = "Investigate"),
	Combat		UMETA(DisplayName = "Combat"),
	Search		UMETA(DisplayName = "Search"),
	Flee		UMETA(DisplayName = "Flee")
};

/**
 * AAnansiAIController
 *
 * Base AI controller for all enemies. Manages perception (sight, hearing,
 * damage), threat assessment, target tracking, alert propagation to nearby
 * AI, and behavior-tree / blackboard integration.
 */
UCLASS()
class ANANSIGAME_API AAnansiAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAnansiAIController();

	// -------------------------------------------------------------------
	// AAIController overrides
	// -------------------------------------------------------------------

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	// -------------------------------------------------------------------
	// State management
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Anansi|AI")
	void SetAIState(EAnansiAIState NewState);

	UFUNCTION(BlueprintPure, Category = "Anansi|AI")
	EAnansiAIState GetAIState() const { return CurrentState; }

	// -------------------------------------------------------------------
	// Target tracking
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|AI")
	AActor* GetCurrentTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Anansi|AI")
	void SetCurrentTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Anansi|AI")
	void ClearTarget();

	// -------------------------------------------------------------------
	// Threat assessment
	// -------------------------------------------------------------------

	/** Evaluate threat level of the given actor (0 = no threat, 1 = maximum). */
	UFUNCTION(BlueprintPure, Category = "Anansi|AI")
	float EvaluateThreatLevel(AActor* Actor) const;

	// -------------------------------------------------------------------
	// Alert propagation
	// -------------------------------------------------------------------

	/** Alert nearby AI controllers within radius about a threat at the given location. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|AI")
	void AlertNearbyAI(const FVector& ThreatLocation, float AlertRadius = 1500.0f);

	/** Called when alerted by another AI controller. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|AI")
	void ReceiveAlert(const FVector& ThreatLocation, AActor* ThreatActor);

protected:
	// -------------------------------------------------------------------
	// Behavior tree
	// -------------------------------------------------------------------

	/** Default behavior tree to run on possession. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|AI")
	TObjectPtr<UBehaviorTree> DefaultBehaviorTree;

	// -------------------------------------------------------------------
	// Perception configuration
	// -------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|AI|Perception")
	float SightRadius = 2000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|AI|Perception")
	float LoseSightRadius = 2500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|AI|Perception")
	float SightFieldOfView = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|AI|Perception")
	float HearingRange = 1500.0f;

	// -------------------------------------------------------------------
	// Blackboard key names
	// -------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|AI|Blackboard")
	FName BB_AIState = FName("AIState");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|AI|Blackboard")
	FName BB_TargetActor = FName("TargetActor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|AI|Blackboard")
	FName BB_LastKnownLocation = FName("LastKnownLocation");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|AI|Blackboard")
	FName BB_InvestigateLocation = FName("InvestigateLocation");

private:
	/** Perception update callback. */
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void ConfigurePerception();
	void UpdateBlackboardState();

	UPROPERTY(VisibleAnywhere, Category = "Anansi|AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	EAnansiAIState CurrentState = EAnansiAIState::Patrol;
};
