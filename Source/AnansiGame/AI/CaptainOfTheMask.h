// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/BossBase.h"
#include "CaptainOfTheMask.generated.h"

/**
 * ACaptainOfTheMask
 *
 * Vertical slice boss: Captain of the Palace Guard.
 *
 * Phase 1 (100%-50% HP):
 *  - Standard melee attacks (sword swings)
 *  - Occasional shield bash (knockback)
 *  - Summons 2 guards at 75% HP
 *
 * Phase 2 (50%-0% HP):
 *  - Removes mask — faster, more aggressive
 *  - Lunging stab attack (dodgeable)
 *  - Ground slam (AoE knockback)
 *  - Summons 3 guards at 25% HP
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ACaptainOfTheMask : public ABossBase
{
	GENERATED_BODY()

public:
	ACaptainOfTheMask();

	virtual void BeginPlay() override;

protected:
	virtual void OnPhaseTransition(int32 NewPhase) override;

	/** Number of guards to summon during phase 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Captain")
	int32 Phase1GuardCount = 2;

	/** Number of guards to summon during phase 2. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Captain")
	int32 Phase2GuardCount = 3;

	/** Radius at which guards are summoned around the boss. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Captain")
	float GuardSummonRadius = 500.0f;

	/** Whether guards have already been summoned in phase 1. */
	bool bPhase1GuardsSummoned = false;

	/** Whether guards have already been summoned in phase 2. */
	bool bPhase2GuardsSummoned = false;

private:
	void SummonGuards(int32 Count);

	UFUNCTION()
	void OnHealthThresholdCheck(float NewHealth, float InMaxHealth);
};
