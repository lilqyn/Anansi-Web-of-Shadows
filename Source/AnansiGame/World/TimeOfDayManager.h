// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimeOfDayManager.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Dawn	UMETA(DisplayName = "Dawn"),
	Day		UMETA(DisplayName = "Day"),
	Dusk	UMETA(DisplayName = "Dusk"),
	Night	UMETA(DisplayName = "Night")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, ETimeOfDay, NewTime);

/**
 * ATimeOfDayManager
 *
 * Controls a day/night cycle by rotating a directional light and
 * adjusting ambient/sky light intensity. Affects stealth — night
 * makes the player harder to detect.
 *
 * Drop one into the level. Uses engine default lights if none assigned.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ATimeOfDayManager : public AActor
{
	GENERATED_BODY()

public:
	ATimeOfDayManager();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Anansi|TimeOfDay")
	ETimeOfDay GetCurrentTimeOfDay() const;

	UFUNCTION(BlueprintPure, Category = "Anansi|TimeOfDay")
	float GetCurrentHour() const { return CurrentHour; }

	/** Set time directly (0-24). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|TimeOfDay")
	void SetTime(float Hour);

	/** Get a stealth modifier based on time of day (lower = harder to detect). */
	UFUNCTION(BlueprintPure, Category = "Anansi|TimeOfDay")
	float GetStealthModifier() const;

	/** Cycle speed — how many in-game hours pass per real second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|TimeOfDay", meta = (ClampMin = "0.0"))
	float TimeScale = 0.5f; // 1 real minute = 30 in-game minutes

	/** Pause the cycle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|TimeOfDay")
	bool bPaused = false;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|TimeOfDay")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDirectionalLightComponent> SunLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UPostProcessComponent> NightPostProcess;

private:
	float CurrentHour = 10.0f; // Start at 10 AM
	ETimeOfDay LastTimeOfDay = ETimeOfDay::Day;

	void UpdateLighting();
};
