// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/AnansiGameMode.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "Player/AnansiPlayerController.h"
#include "Core/AnansiGameState.h"
#include "UI/AnansiDevHUD.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AAnansiGameMode::AAnansiGameMode()
{
	DefaultPawnClass = AAnansiCharacter::StaticClass();
	PlayerControllerClass = AAnansiPlayerController::StaticClass();
	GameStateClass = AAnansiGameState::StaticClass();
	HUDClass = AAnansiDevHUD::StaticClass();
}

void AAnansiGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogAnansi, Log, TEXT("InitGame: Map=%s"), *MapName);
}

void AAnansiGameMode::StartPlay()
{
	Super::StartPlay();

	// Apply the designer-configured initial phase for this level.
	SetGamePhase(InitialPhase);

	UE_LOG(LogAnansi, Log, TEXT("StartPlay: Phase set to %d"), static_cast<int32>(InitialPhase));
}

APawn* AAnansiGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// Use the base implementation which spawns DefaultPawnClass at StartSpot.
	// Override here is a hook for future customisation (e.g., spawning a different
	// pawn based on story progress or multiplayer role).
	return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}

AActor* AAnansiGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Prefer player starts tagged with the current game phase so that
	// designers can place phase-specific spawn points.
	FString DesiredTag;
	switch (CurrentPhase)
	{
	case EAnansiGamePhase::Combat:   DesiredTag = TEXT("Combat");   break;
	case EAnansiGamePhase::Stealth:  DesiredTag = TEXT("Stealth");  break;
	default:                         DesiredTag = TEXT("");          break;
	}

	if (!DesiredTag.IsEmpty())
	{
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			if (It->PlayerStartTag == FName(*DesiredTag))
			{
				return *It;
			}
		}
	}

	// Fall back to the default selection algorithm.
	return Super::ChoosePlayerStart_Implementation(Player);
}

// ---------------------------------------------------------------------------
// Game phase management
// ---------------------------------------------------------------------------

void AAnansiGameMode::SetGamePhase(EAnansiGamePhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		return;
	}

	const EAnansiGamePhase OldPhase = CurrentPhase;
	CurrentPhase = NewPhase;

	UE_LOG(LogAnansi, Log, TEXT("Game phase changed: %d -> %d"),
		static_cast<int32>(OldPhase), static_cast<int32>(NewPhase));

	OnGamePhaseChanged.Broadcast(OldPhase, NewPhase);
}

// ---------------------------------------------------------------------------
// Wave management
// ---------------------------------------------------------------------------

void AAnansiGameMode::StartNextWave()
{
	if (CurrentWaveIndex + 1 >= TotalWaveCount)
	{
		UE_LOG(LogAnansi, Warning, TEXT("StartNextWave called but no more waves remain."));
		return;
	}

	CurrentWaveIndex++;

	// Ensure we are in combat phase when a wave starts.
	SetGamePhase(EAnansiGamePhase::Combat);

	UE_LOG(LogAnansi, Log, TEXT("Wave %d started."), CurrentWaveIndex);
	OnWaveStarted.Broadcast(CurrentWaveIndex);
}

void AAnansiGameMode::OnWaveEnemiesDefeated()
{
	UE_LOG(LogAnansi, Log, TEXT("Wave %d completed."), CurrentWaveIndex);
	OnWaveCompleted.Broadcast(CurrentWaveIndex);

	// If there are more waves, schedule the next one after an intermission.
	if (CurrentWaveIndex + 1 < TotalWaveCount)
	{
		GetWorldTimerManager().SetTimer(
			WaveIntermissionTimerHandle,
			this,
			&AAnansiGameMode::StartNextWave,
			WaveIntermissionDelay,
			false);
	}
	else
	{
		// All waves cleared — return to exploration.
		SetGamePhase(EAnansiGamePhase::Exploration);
	}
}
