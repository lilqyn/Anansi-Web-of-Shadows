// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/SoundManager.h"
#include "AnansiGame.h"
#include "Kismet/GameplayStatics.h"

void USoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SFX_Hit        = TryLoadSound(TEXT("/Game/Audio/SFX/Combat/Hit.Hit"));
	SFX_Whoosh     = TryLoadSound(TEXT("/Game/Audio/SFX/Combat/Whoosh.Whoosh"));
	SFX_Parry      = TryLoadSound(TEXT("/Game/Audio/SFX/Combat/Parry.Parry"));
	SFX_Death      = TryLoadSound(TEXT("/Game/Audio/SFX/Combat/Death.Death"));
	SFX_Jump       = TryLoadSound(TEXT("/Game/Audio/SFX/Traversal/Jump.Jump"));
	SFX_Land       = TryLoadSound(TEXT("/Game/Audio/SFX/Traversal/Land.Land"));
	SFX_WebShoot   = TryLoadSound(TEXT("/Game/Audio/SFX/Traversal/WebShoot.WebShoot"));
	SFX_Pickup     = TryLoadSound(TEXT("/Game/Audio/SFX/UI/Pickup.Pickup"));
	SFX_Checkpoint = TryLoadSound(TEXT("/Game/Audio/SFX/UI/Checkpoint.Checkpoint"));

	UE_LOG(LogAnansi, Log, TEXT("SoundManager: %d/%d sound effects loaded. Import .wav files to the paths above for audio."),
		LoadedCount, 9);
}

USoundBase* USoundManager::TryLoadSound(const TCHAR* Path)
{
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, Path);
	if (Sound)
	{
		LoadedCount++;
	}
	return Sound;
}

void USoundManager::PlaySoundAt(USoundBase* Sound, FVector Location)
{
	if (!Sound) return;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location);
}

void USoundManager::PlayHitSound(FVector Location)        { PlaySoundAt(SFX_Hit, Location); }
void USoundManager::PlayWhooshSound(FVector Location)     { PlaySoundAt(SFX_Whoosh, Location); }
void USoundManager::PlayParrySound(FVector Location)      { PlaySoundAt(SFX_Parry, Location); }
void USoundManager::PlayDeathSound(FVector Location)      { PlaySoundAt(SFX_Death, Location); }
void USoundManager::PlayJumpSound(FVector Location)       { PlaySoundAt(SFX_Jump, Location); }
void USoundManager::PlayLandSound(FVector Location)       { PlaySoundAt(SFX_Land, Location); }
void USoundManager::PlayWebShootSound(FVector Location)   { PlaySoundAt(SFX_WebShoot, Location); }
void USoundManager::PlayPickupSound(FVector Location)     { PlaySoundAt(SFX_Pickup, Location); }
void USoundManager::PlayCheckpointSound(FVector Location) { PlaySoundAt(SFX_Checkpoint, Location); }
