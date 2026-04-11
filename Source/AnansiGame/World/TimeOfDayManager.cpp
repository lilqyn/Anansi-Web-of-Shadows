// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/TimeOfDayManager.h"
#include "AnansiGame.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ATimeOfDayManager::ATimeOfDayManager()
{
	PrimaryActorTick.bCanEverTick = true;

	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	RootComponent = SunLight;

	SunLight->SetIntensity(5.0f);
	SunLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.85f));

	NightPostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("NightPostProcess"));
	NightPostProcess->SetupAttachment(RootComponent);
	NightPostProcess->bUnbound = true;
	NightPostProcess->Priority = 10.0f;
}

void ATimeOfDayManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bPaused) return;

	// Advance time
	CurrentHour += TimeScale * DeltaTime;
	if (CurrentHour >= 24.0f) CurrentHour -= 24.0f;

	UpdateLighting();

	// Check for time-of-day change
	ETimeOfDay Current = GetCurrentTimeOfDay();
	if (Current != LastTimeOfDay)
	{
		LastTimeOfDay = Current;
		OnTimeOfDayChanged.Broadcast(Current);
		UE_LOG(LogAnansi, Log, TEXT("Time of day changed to: %d (hour: %.1f)"),
			static_cast<int32>(Current), CurrentHour);
	}
}

ETimeOfDay ATimeOfDayManager::GetCurrentTimeOfDay() const
{
	if (CurrentHour >= 5.0f && CurrentHour < 8.0f) return ETimeOfDay::Dawn;
	if (CurrentHour >= 8.0f && CurrentHour < 18.0f) return ETimeOfDay::Day;
	if (CurrentHour >= 18.0f && CurrentHour < 21.0f) return ETimeOfDay::Dusk;
	return ETimeOfDay::Night;
}

void ATimeOfDayManager::SetTime(float Hour)
{
	CurrentHour = FMath::Fmod(FMath::Max(0.0f, Hour), 24.0f);
	UpdateLighting();
}

float ATimeOfDayManager::GetStealthModifier() const
{
	switch (GetCurrentTimeOfDay())
	{
	case ETimeOfDay::Night: return 0.4f; // Much harder to detect
	case ETimeOfDay::Dawn:
	case ETimeOfDay::Dusk:  return 0.7f; // Somewhat harder
	default:                return 1.0f; // Normal detection
	}
}

void ATimeOfDayManager::UpdateLighting()
{
	if (!SunLight) return;

	const float SunAngle = (CurrentHour - 6.0f) / 12.0f * 180.0f - 90.0f;
	SunLight->SetWorldRotation(FRotator(SunAngle, -45.0f, 0));

	float Intensity;
	if (CurrentHour >= 7.0f && CurrentHour <= 17.0f)
		Intensity = 5.0f;
	else if (CurrentHour >= 5.0f && CurrentHour < 7.0f)
		Intensity = FMath::Lerp(0.5f, 5.0f, (CurrentHour - 5.0f) / 2.0f);
	else if (CurrentHour > 17.0f && CurrentHour <= 19.0f)
		Intensity = FMath::Lerp(5.0f, 0.5f, (CurrentHour - 17.0f) / 2.0f);
	else
		Intensity = 1.5f; // Night moonlight — still playable

	SunLight->SetIntensity(Intensity);

	FLinearColor SunColor;
	switch (GetCurrentTimeOfDay())
	{
	case ETimeOfDay::Dawn:  SunColor = FLinearColor(1.0f, 0.7f, 0.4f); break;
	case ETimeOfDay::Dusk:  SunColor = FLinearColor(1.0f, 0.5f, 0.3f); break;
	case ETimeOfDay::Night: SunColor = FLinearColor(0.4f, 0.4f, 0.7f); break;
	default:                SunColor = FLinearColor(1.0f, 0.95f, 0.85f); break;
	}
	SunLight->SetLightColor(SunColor);

	// Also control all directional lights in the level (the level's own sun)
	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		if (UDirectionalLightComponent* LC = It->GetComponent())
		{
			LC->SetIntensity(Intensity);
			LC->SetLightColor(SunColor);
			LC->SetWorldRotation(FRotator(SunAngle, LC->GetComponentRotation().Yaw, 0));
		}
	}

	// Control sky light intensity
	for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
	{
		if (USkyLightComponent* SC = It->GetLightComponent())
		{
			const float SkyIntensity = (GetCurrentTimeOfDay() == ETimeOfDay::Night) ? 0.5f :
				(GetCurrentTimeOfDay() == ETimeOfDay::Dawn || GetCurrentTimeOfDay() == ETimeOfDay::Dusk) ? 0.6f : 1.0f;
			SC->SetIntensity(SkyIntensity);
		}
	}

	// Force brightness via post-process exposure and color grading
	if (NightPostProcess)
	{
		const ETimeOfDay TOD = GetCurrentTimeOfDay();

		NightPostProcess->Settings.bOverride_AutoExposureBias = true;
		NightPostProcess->Settings.bOverride_SceneColorTint = true;
		NightPostProcess->Settings.bOverride_VignetteIntensity = true;

		if (TOD == ETimeOfDay::Night)
		{
			// Dark with blue tint
			NightPostProcess->Settings.AutoExposureBias = -3.0f;
			NightPostProcess->Settings.SceneColorTint = FLinearColor(0.6f, 0.65f, 1.0f);
			NightPostProcess->Settings.VignetteIntensity = 0.8f;
		}
		else if (TOD == ETimeOfDay::Dawn)
		{
			NightPostProcess->Settings.AutoExposureBias = -0.5f;
			NightPostProcess->Settings.SceneColorTint = FLinearColor(1.1f, 0.9f, 0.7f);
			NightPostProcess->Settings.VignetteIntensity = 0.3f;
		}
		else if (TOD == ETimeOfDay::Dusk)
		{
			NightPostProcess->Settings.AutoExposureBias = -1.0f;
			NightPostProcess->Settings.SceneColorTint = FLinearColor(1.2f, 0.7f, 0.5f);
			NightPostProcess->Settings.VignetteIntensity = 0.4f;
		}
		else // Day
		{
			NightPostProcess->Settings.AutoExposureBias = 0.0f;
			NightPostProcess->Settings.SceneColorTint = FLinearColor(1.0f, 1.0f, 1.0f);
			NightPostProcess->Settings.VignetteIntensity = 0.0f;
		}
	}
}
