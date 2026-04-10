// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthComponent.generated.h"

/**
 * Player stealth visibility state.
 */
UENUM(BlueprintType)
enum class EStealthState : uint8
{
	Visible			UMETA(DisplayName = "Visible"),
	PartiallyHidden	UMETA(DisplayName = "Partially Hidden"),
	Hidden			UMETA(DisplayName = "Hidden")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnteredStealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDetected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStealthKill, AActor*, Victim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStealthStateChanged, EStealthState, OldState, EStealthState, NewState);

/**
 * UStealthComponent
 *
 * Attached to the player character. Calculates stealth visibility based on
 * light level, movement noise, and cover, and exposes the result to the AI
 * detection system.
 */
UCLASS(ClassGroup = (Anansi), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UStealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStealthComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// -------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Stealth")
	EStealthState GetStealthState() const { return CurrentState; }

	/** Overall visibility score 0-1 (0 = fully hidden, 1 = fully visible). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Stealth")
	float GetVisibilityScore() const { return VisibilityScore; }

	/** Current movement noise level 0-1. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Stealth")
	float GetNoiseLevel() const { return NoiseLevel; }

	/** Light level at the player location 0-1. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Stealth")
	float GetLightLevel() const { return LightLevel; }

	// -------------------------------------------------------------------
	// Stealth takedown
	// -------------------------------------------------------------------

	/** Check if the player can perform a stealth takedown on the given enemy. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Stealth")
	bool CanPerformStealthTakedown(AActor* Enemy) const;

	/** Execute a stealth takedown. Fires OnStealthKill. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Stealth")
	void PerformStealthTakedown(AActor* Enemy);

	// -------------------------------------------------------------------
	// Cover
	// -------------------------------------------------------------------

	/** Manually set a cover bonus (called by CoverVolume on overlap). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Stealth")
	void SetCoverBonus(float Bonus);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Stealth")
	void ClearCoverBonus();

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Stealth")
	FOnEnteredStealth OnEnteredStealth;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Stealth")
	FOnDetected OnDetected;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Stealth")
	FOnStealthKill OnStealthKill;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Stealth")
	FOnStealthStateChanged OnStealthStateChanged;

protected:
	// -------------------------------------------------------------------
	// Configuration
	// -------------------------------------------------------------------

	/** Noise generated while sprinting (0-1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Stealth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SprintNoiseLevel = 0.8f;

	/** Noise generated while walking (0-1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Stealth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WalkNoiseLevel = 0.3f;

	/** Noise generated while crouching (0-1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Stealth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CrouchNoiseLevel = 0.05f;

	/** Visibility threshold below which the player is Hidden. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Stealth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HiddenThreshold = 0.25f;

	/** Visibility threshold below which the player is Partially Hidden. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Stealth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PartiallyHiddenThreshold = 0.55f;

	/** Maximum angle (degrees) behind an enemy for a valid takedown. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Stealth", meta = (ClampMin = "0.0"))
	float TakedownAngleThreshold = 60.0f;

	/** Maximum distance for a stealth takedown. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Stealth", meta = (ClampMin = "0.0"))
	float TakedownRange = 200.0f;

private:
	void UpdateLightLevel();
	void UpdateNoiseLevel();
	void UpdateVisibilityScore();
	void UpdateStealthState();

	EStealthState CurrentState = EStealthState::Visible;
	float VisibilityScore = 1.0f;
	float NoiseLevel = 0.0f;
	float LightLevel = 1.0f;
	float CoverBonusValue = 0.0f;
};
