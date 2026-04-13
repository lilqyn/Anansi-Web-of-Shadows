// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveSurvival.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveSurvivalWave, int32, WaveNumber, int32, EnemyCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveSurvivalEnd, int32, TotalWaves);

/**
 * AWaveSurvival
 *
 * Endless wave survival mode. Each wave spawns more and harder enemies.
 * Difficulty escalates: more enemies, mixed types, faster spawns.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AWaveSurvival : public AActor
{
	GENERATED_BODY()

public:
	AWaveSurvival();

	UFUNCTION(BlueprintCallable, Category = "Anansi|Survival")
	void StartSurvival();

	UFUNCTION(BlueprintPure, Category = "Anansi|Survival")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Survival")
	int32 GetAliveCount() const;

	UFUNCTION(BlueprintPure, Category = "Anansi|Survival")
	bool IsActive() const { return bIsActive; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Survival", meta = (ClampMin = "1"))
	int32 BaseEnemiesPerWave = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Survival", meta = (ClampMin = "0.0"))
	float TimeBetweenWaves = 5.0f;

	UPROPERTY(BlueprintAssignable) FOnWaveSurvivalWave OnWaveStarted;
	UPROPERTY(BlueprintAssignable) FOnWaveSurvivalEnd OnSurvivalEnded;

private:
	int32 CurrentWave = 0;
	bool bIsActive = false;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SpawnedEnemies;

	void SpawnWave();
	void CheckWaveComplete();

	FTimerHandle WaveCheckTimer;
	FTimerHandle WaveDelayTimer;
};
