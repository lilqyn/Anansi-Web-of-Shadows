// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AnansiGameMode.generated.h"

/**
 * Gameplay phase — determines which systems are active and how the camera/HUD behaves.
 * Transitions are managed by the game mode and broadcast to all listeners.
 */
UENUM(BlueprintType)
enum class EAnansiGamePhase : uint8
{
	/** Free movement and exploration, puzzles accessible. */
	Exploration		UMETA(DisplayName = "Exploration"),

	/** Active combat encounter — enemy AI engaged, combo system active. */
	Combat			UMETA(DisplayName = "Combat"),

	/** Stealth section — detection meters active, instant-kill takedowns available. */
	Stealth			UMETA(DisplayName = "Stealth"),

	/** Non-interactive narrative sequence with camera control. */
	Cutscene		UMETA(DisplayName = "Cutscene"),

	/** Environmental puzzle — special interaction prompts active. */
	Puzzle			UMETA(DisplayName = "Puzzle")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGamePhaseChanged, EAnansiGamePhase, OldPhase, EAnansiGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCompleted, int32, WaveIndex);

/**
 * AAnansiGameMode
 *
 * Manages wave/encounter states, player spawn logic, and game phase transitions.
 * Each level can configure its starting phase and wave definitions via data assets
 * or level-blueprint overrides.
 */
UCLASS()
class ANANSIGAME_API AAnansiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAnansiGameMode();

	// -----------------------------------------------------------------------
	// AGameModeBase overrides
	// -----------------------------------------------------------------------

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// -----------------------------------------------------------------------
	// Game phase management
	// -----------------------------------------------------------------------

	/** Transition to a new game phase. Broadcasts OnGamePhaseChanged. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|GameMode")
	void SetGamePhase(EAnansiGamePhase NewPhase);

	/** Returns the current game phase. */
	UFUNCTION(BlueprintPure, Category = "Anansi|GameMode")
	EAnansiGamePhase GetCurrentGamePhase() const { return CurrentPhase; }

	// -----------------------------------------------------------------------
	// Wave / encounter management
	// -----------------------------------------------------------------------

	/** Begin the next combat wave. Called automatically or manually from level scripts. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|GameMode")
	void StartNextWave();

	/** Called when all enemies in the current wave are defeated. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|GameMode")
	void OnWaveEnemiesDefeated();

	/** Get the current wave index (0-based). */
	UFUNCTION(BlueprintPure, Category = "Anansi|GameMode")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	/** Get the total number of waves configured for this encounter. */
	UFUNCTION(BlueprintPure, Category = "Anansi|GameMode")
	int32 GetTotalWaveCount() const { return TotalWaveCount; }

	// -----------------------------------------------------------------------
	// Delegates
	// -----------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|GameMode")
	FOnGamePhaseChanged OnGamePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|GameMode")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|GameMode")
	FOnWaveCompleted OnWaveCompleted;

protected:
	/** The phase the level starts in — set per-level in the game mode blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|GameMode")
	EAnansiGamePhase InitialPhase = EAnansiGamePhase::Exploration;

	/** Total waves configured for the current encounter. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anansi|Waves")
	int32 TotalWaveCount = 0;

	/** Delay between waves in seconds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anansi|Waves", meta = (ClampMin = "0.0"))
	float WaveIntermissionDelay = 3.0f;

private:
	EAnansiGamePhase CurrentPhase = EAnansiGamePhase::Exploration;
	int32 CurrentWaveIndex = -1;

	FTimerHandle WaveIntermissionTimerHandle;
};
