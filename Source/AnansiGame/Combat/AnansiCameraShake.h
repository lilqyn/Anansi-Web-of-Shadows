// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "AnansiCameraShake.generated.h"

/**
 * UAnansiShakePattern — simple noise shake pattern.
 */
UCLASS()
class ANANSIGAME_API UAnansiShakePattern : public UCameraShakePattern
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Shake")
	float Duration = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Shake")
	float Amplitude = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Shake")
	float Frequency = 25.0f;

protected:
	virtual void GetShakePatternInfoImpl(FCameraShakeInfo& OutInfo) const override;
	virtual void StartShakePatternImpl(const FCameraShakePatternStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult) override;
	virtual bool IsFinishedImpl() const override;

private:
	float ElapsedTime = 0.0f;
};

/** Light camera shake for dealing damage. */
UCLASS()
class ANANSIGAME_API UAnansiHitShake : public UCameraShakeBase
{
	GENERATED_BODY()
public:
	UAnansiHitShake(const FObjectInitializer& OI);
};

/** Heavier shake for taking damage. */
UCLASS()
class ANANSIGAME_API UAnansiDamageShake : public UCameraShakeBase
{
	GENERATED_BODY()
public:
	UAnansiDamageShake(const FObjectInitializer& OI);
};
