// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WebSwingPoint.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * AWebSwingPoint
 *
 * A world-placed anchor for web swinging. The TraversalComponent scans for
 * these when the player is airborne and initiates pendulum-based swing
 * physics on attachment.
 */
UCLASS()
class ANANSIGAME_API AWebSwingPoint : public AActor
{
	GENERATED_BODY()

public:
	AWebSwingPoint();

	// -------------------------------------------------------------------
	// Swing properties
	// -------------------------------------------------------------------

	/** Maximum rope length when attaching from below. */
	UFUNCTION(BlueprintPure, Category = "Anansi|WebSwing")
	float GetSwingRadius() const { return SwingRadius; }

	/** Preferred entry angle range (degrees from vertical). */
	UFUNCTION(BlueprintPure, Category = "Anansi|WebSwing")
	float GetMaxEntryAngle() const { return MaxEntryAngle; }

	/** Preferred exit angle range (degrees from vertical). */
	UFUNCTION(BlueprintPure, Category = "Anansi|WebSwing")
	float GetMaxExitAngle() const { return MaxExitAngle; }

	/** Is this point currently usable? (can be disabled by puzzles, etc.) */
	UFUNCTION(BlueprintPure, Category = "Anansi|WebSwing")
	bool IsActive() const { return bIsActive; }

	UFUNCTION(BlueprintCallable, Category = "Anansi|WebSwing")
	void SetActive(bool bActive);

	/** Check if a character at the given location can attach to this point. */
	UFUNCTION(BlueprintPure, Category = "Anansi|WebSwing")
	bool CanAttachFrom(const FVector& CharacterLocation) const;

	/** Calculate the release velocity for a character releasing at the given angle. */
	UFUNCTION(BlueprintPure, Category = "Anansi|WebSwing")
	FVector CalculateReleaseVelocity(float CurrentAngle, float AngularVelocity) const;

protected:
	// -------------------------------------------------------------------
	// Configuration
	// -------------------------------------------------------------------

	/** Maximum rope length when swinging from this point. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anansi|WebSwing", meta = (ClampMin = "100.0"))
	float SwingRadius = 800.0f;

	/** Maximum entry angle in degrees from directly below. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anansi|WebSwing", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxEntryAngle = 60.0f;

	/** Maximum exit angle in degrees from directly below. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anansi|WebSwing", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxExitAngle = 70.0f;

	/** Detection radius for auto-grab when airborne. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anansi|WebSwing", meta = (ClampMin = "100.0"))
	float AutoDetectionRadius = 1500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|WebSwing")
	TObjectPtr<USphereComponent> DetectionVolume;

	/** Visual representation of the swing point. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|WebSwing")
	TObjectPtr<UStaticMeshComponent> PointMesh;

private:
	bool bIsActive = true;
};
