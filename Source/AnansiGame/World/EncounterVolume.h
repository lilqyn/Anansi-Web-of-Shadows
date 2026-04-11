// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EncounterVolume.generated.h"

class UBoxComponent;
class AAnansiCharacter;

/**
 * Spawn slot — defines one enemy to spawn with a location and class.
 */
USTRUCT(BlueprintType)
struct FEnemySpawnSlot
{
	GENERATED_BODY()

	/** Enemy class to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<APawn> EnemyClass;

	/** Relative offset from the encounter volume origin. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector SpawnOffset = FVector::ZeroVector;

	/** Rotation for the spawned enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FRotator SpawnRotation = FRotator::ZeroRotator;
};

/**
 * Wave definition — a group of enemies that spawn together.
 */
USTRUCT(BlueprintType)
struct FEncounterWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FEnemySpawnSlot> Enemies;

	/** Delay before this wave spawns (after trigger or previous wave). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "0.0"))
	float SpawnDelay = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEncounterStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEncounterCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEncounterWaveStarted, int32, WaveIndex);

/**
 * AEncounterVolume
 *
 * Trigger volume that spawns waves of enemies when the player enters.
 * Tracks alive enemies per wave and auto-advances when all are defeated.
 * Broadcasts events for game mode, HUD, and music integration.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AEncounterVolume : public AActor
{
	GENERATED_BODY()

public:
	AEncounterVolume();

	/** Manually trigger the encounter (ignoring the overlap volume). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Encounter")
	void StartEncounter();

	/** Check if the encounter is currently active. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Encounter")
	bool IsEncounterActive() const { return bIsActive; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Encounter")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Encounter")
	int32 GetAliveEnemyCount() const { return AliveEnemies.Num(); }

	// -- Configuration -------------------------------------------------------

	/** The waves of enemies for this encounter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Encounter")
	TArray<FEncounterWave> Waves;

	/** If true, the encounter triggers automatically when the player enters the volume. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Encounter")
	bool bAutoTrigger = true;

	/** If true, the encounter can only be triggered once. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Encounter")
	bool bOneShot = true;

	// -- Delegates ------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Encounter")
	FOnEncounterStarted OnEncounterStarted;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Encounter")
	FOnEncounterCompleted OnEncounterCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Encounter")
	FOnEncounterWaveStarted OnEncounterWaveStarted;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Encounter")
	TObjectPtr<UBoxComponent> TriggerBox;

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bIsActive = false;
	bool bHasTriggered = false;
	int32 CurrentWaveIndex = -1;

	UPROPERTY()
	TArray<TWeakObjectPtr<APawn>> AliveEnemies;

	void SpawnWave(int32 WaveIndex);
	void OnEnemyDestroyed(AActor* DestroyedActor);
	void CheckWaveCompletion();
	void AdvanceToNextWave();

	FTimerHandle WaveDelayTimerHandle;
};
