// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DetectionComponent.generated.h"

class UStealthComponent;
class UWidgetComponent;

/**
 * UDetectionComponent
 *
 * Attached to enemies. Performs visual (cone) and audio (radius) detection of
 * the player, maintains an awareness meter (0-100), and exposes the last
 * known player position. Includes a visual awareness indicator above the
 * enemy's head.
 */
UCLASS(ClassGroup = (Anansi), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDetectionComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// -------------------------------------------------------------------
	// Awareness
	// -------------------------------------------------------------------

	/** Current awareness value (0-100). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Detection")
	float GetAwareness() const { return Awareness; }

	/** Is the enemy unaware (awareness < suspicious threshold)? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Detection")
	bool IsUnaware() const { return Awareness < SuspiciousThreshold; }

	/** Is the enemy suspicious? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Detection")
	bool IsSuspicious() const { return Awareness >= SuspiciousThreshold && Awareness < AlertThreshold; }

	/** Is the enemy alert? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Detection")
	bool IsAlert() const { return Awareness >= AlertThreshold && Awareness < CombatThreshold; }

	/** Is the enemy in combat awareness? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Detection")
	bool IsInCombat() const { return Awareness >= CombatThreshold; }

	/** Manually increase awareness (e.g. from taking damage). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Detection")
	void AddAwareness(float Amount);

	/** Force full awareness (e.g. from alert propagation). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Detection")
	void SetFullAwareness();

	/** Reset awareness to zero. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Detection")
	void ResetAwareness();

	// -------------------------------------------------------------------
	// Last known position
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Detection")
	FVector GetLastKnownPlayerPosition() const { return LastKnownPlayerPosition; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Detection")
	bool HasLastKnownPosition() const { return bHasLastKnownPosition; }

protected:
	// -------------------------------------------------------------------
	// Visual detection
	// -------------------------------------------------------------------

	/** Maximum sight distance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|Visual")
	float SightRange = 2000.0f;

	/** Half-angle of the vision cone (degrees). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|Visual", meta = (ClampMin = "1.0", ClampMax = "180.0"))
	float SightHalfAngle = 45.0f;

	/** Awareness gained per second when the player is fully visible in the cone. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|Visual", meta = (ClampMin = "0.0"))
	float VisualAwarenessRate = 40.0f;

	// -------------------------------------------------------------------
	// Audio detection
	// -------------------------------------------------------------------

	/** Hearing radius. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|Audio")
	float HearingRange = 1200.0f;

	/** Awareness gained per second from audible player noise. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|Audio", meta = (ClampMin = "0.0"))
	float AudioAwarenessRate = 20.0f;

	// -------------------------------------------------------------------
	// Awareness thresholds
	// -------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|Awareness")
	float SuspiciousThreshold = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|Awareness")
	float AlertThreshold = 70.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|Awareness")
	float CombatThreshold = 100.0f;

	/** Awareness decay rate per second when there is no stimulus. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|Awareness", meta = (ClampMin = "0.0"))
	float AwarenessDecayRate = 10.0f;

	/** Delay before awareness starts decaying after losing stimulus (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|Awareness", meta = (ClampMin = "0.0"))
	float DecayDelay = 3.0f;

	// -------------------------------------------------------------------
	// Indicator
	// -------------------------------------------------------------------

	/** Widget class for the awareness indicator above the enemy's head. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Detection|UI")
	TSubclassOf<UUserWidget> AwarenessIndicatorWidgetClass;

private:
	/** Perform visual detection check against the player. */
	float EvaluateVisualDetection(const ACharacter* Player, const UStealthComponent* Stealth) const;

	/** Perform audio detection check against the player. */
	float EvaluateAudioDetection(const ACharacter* Player, const UStealthComponent* Stealth) const;

	/** Update the awareness indicator widget. */
	void UpdateIndicator();

	float Awareness = 0.0f;
	float DecayTimer = 0.0f;
	bool bHasStimulus = false;

	FVector LastKnownPlayerPosition = FVector::ZeroVector;
	bool bHasLastKnownPosition = false;

	UPROPERTY()
	TObjectPtr<UWidgetComponent> IndicatorWidgetComp;
};
