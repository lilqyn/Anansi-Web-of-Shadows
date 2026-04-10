// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/AnansiGameInstance.h"
#include "Core/AnansiSaveGame.h"
#include "AnansiGame.h"
#include "Kismet/GameplayStatics.h"

UAnansiGameInstance::UAnansiGameInstance()
{
}

void UAnansiGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogAnansi, Log, TEXT("AnansiGameInstance initialised."));
}

// ---------------------------------------------------------------------------
// Save / Load
// ---------------------------------------------------------------------------

bool UAnansiGameInstance::SaveGame(const FString& SlotName, int32 UserIndex)
{
	if (!CurrentSaveData)
	{
		CurrentSaveData = Cast<UAnansiSaveGame>(
			UGameplayStatics::CreateSaveGameObject(UAnansiSaveGame::StaticClass()));
	}

	// Stamp play-time before saving.
	CurrentSaveData->TotalPlayTimeSeconds = TotalPlayTimeSeconds;

	if (UGameplayStatics::SaveGameToSlot(CurrentSaveData, SlotName, UserIndex))
	{
		UE_LOG(LogAnansi, Log, TEXT("Game saved to slot '%s'."), *SlotName);
		return true;
	}

	UE_LOG(LogAnansi, Warning, TEXT("Failed to save game to slot '%s'."), *SlotName);
	return false;
}

bool UAnansiGameInstance::LoadGame(const FString& SlotName, int32 UserIndex)
{
	if (!DoesSaveExist(SlotName, UserIndex))
	{
		UE_LOG(LogAnansi, Warning, TEXT("No save in slot '%s'."), *SlotName);
		return false;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
	CurrentSaveData = Cast<UAnansiSaveGame>(Loaded);

	if (CurrentSaveData)
	{
		TotalPlayTimeSeconds = CurrentSaveData->TotalPlayTimeSeconds;
		UE_LOG(LogAnansi, Log, TEXT("Game loaded from slot '%s'."), *SlotName);
		return true;
	}

	UE_LOG(LogAnansi, Warning, TEXT("Failed to cast loaded save in slot '%s'."), *SlotName);
	return false;
}

bool UAnansiGameInstance::DeleteSave(const FString& SlotName, int32 UserIndex)
{
	if (UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex))
	{
		UE_LOG(LogAnansi, Log, TEXT("Deleted save slot '%s'."), *SlotName);

		if (CurrentSaveData)
		{
			CurrentSaveData = nullptr;
		}
		return true;
	}
	return false;
}

bool UAnansiGameInstance::DoesSaveExist(const FString& SlotName, int32 UserIndex) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

// ---------------------------------------------------------------------------
// Audio settings
// ---------------------------------------------------------------------------

void UAnansiGameInstance::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAnansiGameInstance::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAnansiGameInstance::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}
