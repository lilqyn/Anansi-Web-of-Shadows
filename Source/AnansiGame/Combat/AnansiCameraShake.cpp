// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Combat/AnansiCameraShake.h"

// ---------------------------------------------------------------------------
// Shake Pattern
// ---------------------------------------------------------------------------

void UAnansiShakePattern::GetShakePatternInfoImpl(FCameraShakeInfo& OutInfo) const
{
	OutInfo.Duration = FCameraShakeDuration(Duration);
}

void UAnansiShakePattern::StartShakePatternImpl(const FCameraShakePatternStartParams& Params)
{
	ElapsedTime = 0.0f;
}

void UAnansiShakePattern::UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params,
	FCameraShakePatternUpdateResult& OutResult)
{
	ElapsedTime += Params.DeltaTime;

	const float Alpha = FMath::Clamp(1.0f - (ElapsedTime / Duration), 0.0f, 1.0f);
	const float Noise = FMath::Sin(ElapsedTime * Frequency) * Amplitude * Alpha;

	OutResult.Location = FVector(
		FMath::FRandRange(-1.0f, 1.0f) * Noise,
		FMath::FRandRange(-1.0f, 1.0f) * Noise,
		0.0f);
	OutResult.Rotation = FRotator(Noise * 0.3f, Noise * 0.2f, 0.0f);
}

bool UAnansiShakePattern::IsFinishedImpl() const
{
	return ElapsedTime >= Duration;
}

// ---------------------------------------------------------------------------
// Shake Classes
// ---------------------------------------------------------------------------

UAnansiHitShake::UAnansiHitShake(const FObjectInitializer& OI)
	: Super(OI.SetDefaultSubobjectClass<UAnansiShakePattern>(TEXT("RootShakePattern")))
{
	if (UAnansiShakePattern* Pattern = Cast<UAnansiShakePattern>(GetRootShakePattern()))
	{
		Pattern->Duration = 0.12f;
		Pattern->Amplitude = 2.5f;
		Pattern->Frequency = 25.0f;
	}
}

UAnansiDamageShake::UAnansiDamageShake(const FObjectInitializer& OI)
	: Super(OI.SetDefaultSubobjectClass<UAnansiShakePattern>(TEXT("RootShakePattern")))
{
	if (UAnansiShakePattern* Pattern = Cast<UAnansiShakePattern>(GetRootShakePattern()))
	{
		Pattern->Duration = 0.2f;
		Pattern->Amplitude = 5.0f;
		Pattern->Frequency = 30.0f;
	}
}
