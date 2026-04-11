// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyBase.h"
#include "BossBase.generated.h"

class USimpleEnemyAI;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossDefeated);

/**
 * ABossBase
 *
 * Base class for boss encounters. Supports multi-phase fights with
 * health thresholds, phase-specific attacks, and HUD integration.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ABossBase : public AEnemyBase
{
	GENERATED_BODY()

public:
	ABossBase();

	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Anansi|Boss")
	int32 GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Boss")
	int32 GetTotalPhases() const { return PhaseHealthThresholds.Num() + 1; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Boss")
	FText GetBossName() const { return BossDisplayName; }

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Boss")
	FOnBossPhaseChanged OnBossPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Boss")
	FOnBossDefeated OnBossDefeated;

protected:
	/** Display name shown on the boss health bar. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Boss")
	FText BossDisplayName = NSLOCTEXT("Anansi", "BossDefault", "Boss");

	/**
	 * Health percentages at which phase transitions occur.
	 * E.g., {0.5f} means phase 2 starts at 50% HP (2-phase fight).
	 * {0.66f, 0.33f} means 3-phase fight.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Boss")
	TArray<float> PhaseHealthThresholds;

	/** Called when a phase transition occurs. Override for phase-specific setup. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Boss")
	virtual void OnPhaseTransition(int32 NewPhase);

	/** AI component for combat behavior. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Boss")
	TObjectPtr<USimpleEnemyAI> BossAI;

private:
	int32 CurrentPhase = 1;
};
