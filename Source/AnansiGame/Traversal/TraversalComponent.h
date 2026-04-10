// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TraversalComponent.generated.h"

class AWebSwingPoint;

/**
 * Traversal movement states.
 */
UENUM(BlueprintType)
enum class ETraversalState : uint8
{
	Grounded	UMETA(DisplayName = "Grounded"),
	WallRunning	UMETA(DisplayName = "Wall Running"),
	WebSwinging	UMETA(DisplayName = "Web Swinging"),
	Climbing	UMETA(DisplayName = "Climbing"),
	Mantling	UMETA(DisplayName = "Mantling"),
	LedgeHanging UMETA(DisplayName = "Ledge Hanging")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTraversalStateChanged, ETraversalState, OldState, ETraversalState, NewState);

/**
 * UTraversalComponent
 *
 * Manages advanced traversal mechanics: wall running, web swinging (pendulum
 * physics), ledge grabbing, climbing, and mantling. Drives a state machine
 * that coordinates with the character movement component.
 */
UCLASS(ClassGroup = (Anansi), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTraversalComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// -------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Traversal")
	ETraversalState GetTraversalState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Traversal")
	bool IsGrounded() const { return CurrentState == ETraversalState::Grounded; }

	// -------------------------------------------------------------------
	// Wall running
	// -------------------------------------------------------------------

	/** Attempt to initiate a wall run. Returns true if conditions are met. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Traversal")
	bool TryWallRun();

	/** Cancel the current wall run. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Traversal")
	void EndWallRun();

	// -------------------------------------------------------------------
	// Web swinging
	// -------------------------------------------------------------------

	/** Attempt to attach to the nearest swing point. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Traversal")
	bool TryWebSwing();

	/** Release from swing and preserve momentum. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Traversal")
	void ReleaseWebSwing();

	/** Apply player input to the swing arc. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Traversal")
	void ApplySwingInput(float ForwardInput, float RightInput);

	// -------------------------------------------------------------------
	// Ledge / mantle
	// -------------------------------------------------------------------

	/** Check for and grab a ledge. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Traversal")
	bool TryLedgeGrab();

	/** Climb up from ledge hang. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Traversal")
	void ClimbUpFromLedge();

	/** Attempt a mantle over a low obstacle. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Traversal")
	bool TryMantle();

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Traversal")
	FOnTraversalStateChanged OnTraversalStateChanged;

protected:
	// -------------------------------------------------------------------
	// Wall run configuration
	// -------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|WallRun")
	float WallRunMaxDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|WallRun")
	float WallRunCooldown = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|WallRun")
	float WallRunSpeed = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|WallRun")
	float WallRunGravityScale = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|WallRun")
	float WallDetectionDistance = 80.0f;

	// -------------------------------------------------------------------
	// Web swing configuration
	// -------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|WebSwing")
	float SwingPointDetectionRadius = 2000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|WebSwing")
	float SwingGravity = 980.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|WebSwing")
	float SwingInputForce = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|WebSwing")
	float SwingDamping = 0.98f;

	// -------------------------------------------------------------------
	// Ledge / mantle configuration
	// -------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|Ledge")
	float LedgeDetectionDistance = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|Ledge")
	float LedgeGrabHeight = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|Mantle")
	float MantleMaxHeight = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Traversal|Mantle")
	TObjectPtr<UAnimMontage> MantleMontage;

private:
	void SetTraversalState(ETraversalState NewState);

	// Wall run helpers
	bool DetectWall(FHitResult& OutHit) const;
	void TickWallRun(float DeltaTime);

	// Swing helpers
	AWebSwingPoint* FindNearestSwingPoint() const;
	void TickWebSwing(float DeltaTime);
	FVector CalculateSwingVelocity() const;

	// Ledge helpers
	bool DetectLedge(FVector& OutLedgeLocation, FVector& OutLedgeNormal) const;

	ETraversalState CurrentState = ETraversalState::Grounded;

	// Wall run state
	float WallRunTimer = 0.0f;
	float WallRunCooldownTimer = 0.0f;
	FVector WallNormal = FVector::ZeroVector;
	bool bWallRunRight = true;

	// Swing state
	UPROPERTY()
	TObjectPtr<AWebSwingPoint> ActiveSwingPoint;
	float SwingAngle = 0.0f;
	float SwingAngularVelocity = 0.0f;
	float SwingRopeLength = 0.0f;

	// Ledge state
	FVector LedgeLocation = FVector::ZeroVector;
	FVector LedgeNormal = FVector::ZeroVector;
};
