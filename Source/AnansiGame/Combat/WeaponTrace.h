// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponTrace.generated.h"

/**
 * Payload broadcast when the weapon trace detects a new hit.
 */
USTRUCT(BlueprintType)
struct FWeaponHitResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "WeaponTrace")
	TWeakObjectPtr<AActor> HitActor;

	UPROPERTY(BlueprintReadOnly, Category = "WeaponTrace")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "WeaponTrace")
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category = "WeaponTrace")
	FName HitBoneName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponHit, const FWeaponHitResult&, HitResult);

/**
 * UWeaponTraceComponent
 *
 * Attached to the player character (or enemy). Performs per-frame sphere traces
 * between two skeletal-mesh sockets (base and tip) during attack animations to
 * detect hit targets. Tracks which actors have already been hit during the
 * current swing to prevent multi-hit on the same target.
 *
 * Usage:
 *  1. Configure StartSocketName and EndSocketName to match weapon bone sockets.
 *  2. Call StartTrace() at the beginning of an attack's active frames (anim notify).
 *  3. Call EndTrace() when active frames end.
 *  4. Listen to OnWeaponHit for hit events.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UWeaponTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponTraceComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// -----------------------------------------------------------------------
	// API
	// -----------------------------------------------------------------------

	/** Begin tracing. Clears the already-hit set. */
	UFUNCTION(BlueprintCallable, Category = "WeaponTrace")
	void StartTrace();

	/** Stop tracing. */
	UFUNCTION(BlueprintCallable, Category = "WeaponTrace")
	void EndTrace();

	/** Is the trace currently active? */
	UFUNCTION(BlueprintPure, Category = "WeaponTrace")
	bool IsTracing() const { return bIsTracing; }

	// -----------------------------------------------------------------------
	// Configuration
	// -----------------------------------------------------------------------

	/** Socket name on the owning skeletal mesh marking the weapon base. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponTrace|Config")
	FName StartSocketName = FName("weapon_base");

	/** Socket name on the owning skeletal mesh marking the weapon tip. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponTrace|Config")
	FName EndSocketName = FName("weapon_tip");

	/** Radius of the sphere trace. Larger values are more forgiving. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponTrace|Config", meta = (ClampMin = "1.0"))
	float TraceRadius = 15.0f;

	/** Number of interpolation steps between previous and current socket positions.
	 *  Higher values catch fast swings but cost more traces. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponTrace|Config", meta = (ClampMin = "1", ClampMax = "10"))
	int32 InterpolationSteps = 3;

	/** Collision channel used for the sweep. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponTrace|Config")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	/** Whether to draw debug shapes in non-shipping builds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponTrace|Debug")
	bool bDrawDebug = false;

	// -----------------------------------------------------------------------
	// Events
	// -----------------------------------------------------------------------

	/** Fired each time a new actor is hit during the current swing. */
	UPROPERTY(BlueprintAssignable, Category = "WeaponTrace")
	FOnWeaponHit OnWeaponHit;

private:
	/** Perform the actual sweep between two world-space points. */
	void PerformSweep(const FVector& Start, const FVector& End);

	/** Get socket world location from the owner's mesh. Returns false if socket not found. */
	bool GetSocketLocation(FName SocketName, FVector& OutLocation) const;

	bool bIsTracing = false;

	/** Actors already hit during this trace pass — prevents double-hitting. */
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;

	/** Previous frame socket positions for interpolation. */
	FVector PrevStartLocation = FVector::ZeroVector;
	FVector PrevEndLocation = FVector::ZeroVector;
	bool bHasPreviousPositions = false;
};
