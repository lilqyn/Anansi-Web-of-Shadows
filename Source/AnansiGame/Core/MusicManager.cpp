// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/MusicManager.h"
#include "AnansiGame.h"
#include "AI/EnemyBase.h"
#include "AI/BossBase.h"
#include "Player/AnansiCharacter.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

void UMusicManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Track_Exploration = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Music/Exploration.Exploration"));
	Track_Combat      = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Music/Combat.Combat"));
	Track_Boss        = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Music/Boss.Boss"));
	Track_Stealth     = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Music/Stealth.Stealth"));

	const int32 Loaded = (Track_Exploration ? 1 : 0) + (Track_Combat ? 1 : 0) +
		(Track_Boss ? 1 : 0) + (Track_Stealth ? 1 : 0);
	UE_LOG(LogAnansi, Log, TEXT("MusicManager: %d/4 music tracks loaded. Import .wav files to /Game/Audio/Music/"), Loaded);
}

TStatId UMusicManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMusicManager, STATGROUP_Tickables);
}

void UMusicManager::Tick(float DeltaTime)
{
	StateCheckTimer += DeltaTime;
	if (StateCheckTimer >= 1.0f)
	{
		StateCheckTimer = 0.0f;
		CheckGameplayState();
	}
}

void UMusicManager::CheckGameplayState()
{
	UWorld* World = GetWorld();
	if (!World) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!Player) return;

	// Check for nearby bosses first (highest priority)
	for (TActorIterator<ABossBase> It(World); It; ++It)
	{
		if (!It->IsDead())
		{
			const float Dist = FVector::Dist(Player->GetActorLocation(), It->GetActorLocation());
			if (Dist < 3000.0f)
			{
				SetMusicState(EMusicState::Boss);
				return;
			}
		}
	}

	// Check for enemies in combat awareness
	bool bInCombat = false;
	for (TActorIterator<AEnemyBase> It(World); It; ++It)
	{
		if (It->IsDead()) continue;
		if (It->GetAwarenessState() == EEnemyAwareness::Combat)
		{
			const float Dist = FVector::Dist(Player->GetActorLocation(), It->GetActorLocation());
			if (Dist < 2500.0f)
			{
				bInCombat = true;
				break;
			}
		}
	}

	// Check player stealth state
	if (AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(Player))
	{
		if (Anansi->GetCharacterState() == EAnansiCharacterState::Crouching && !bInCombat)
		{
			SetMusicState(EMusicState::Stealth);
			return;
		}
	}

	SetMusicState(bInCombat ? EMusicState::Combat : EMusicState::Exploration);
}

void UMusicManager::SetMusicState(EMusicState NewState)
{
	if (CurrentState == NewState) return;

	CurrentState = NewState;
	UE_LOG(LogAnansi, Verbose, TEXT("Music state: %d"), static_cast<int32>(NewState));

	// Stop current track
	if (ActiveAudioComp && ActiveAudioComp->IsPlaying())
	{
		ActiveAudioComp->FadeOut(1.0f, 0.0f);
	}

	// Start new track
	USoundBase* Track = GetTrackForState(NewState);
	if (Track)
	{
		ActiveAudioComp = UGameplayStatics::CreateSound2D(GetWorld(), Track, 1.0f, 1.0f, 0.0f, nullptr, true);
		if (ActiveAudioComp)
		{
			ActiveAudioComp->FadeIn(1.5f, 1.0f);
		}
	}
}

USoundBase* UMusicManager::GetTrackForState(EMusicState State) const
{
	switch (State)
	{
	case EMusicState::Exploration: return Track_Exploration;
	case EMusicState::Combat:      return Track_Combat;
	case EMusicState::Boss:        return Track_Boss;
	case EMusicState::Stealth:     return Track_Stealth;
	default:                       return nullptr;
	}
}
