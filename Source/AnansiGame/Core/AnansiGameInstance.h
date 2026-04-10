// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AnansiGameInstance.generated.h"

class UAnansiSaveGame;

/**
 * UAnansiGameInstance
 *
 * Persists across level transitions. Owns save/load logic, global audio settings,
 * and any data that must survive map travel (unlocked abilities, collected story
 * fragments, player preferences).
 */
UCLASS()
class ANANSIGAME_API UAnansiGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UAnansiGameInstance();

	virtual void Init() override;

	// -----------------------------------------------------------------------
	// Save / Load
	// -----------------------------------------------------------------------

	/** Save current game state to the given slot. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Save")
	bool SaveGame(const FString& SlotName = TEXT("Default"), int32 UserIndex = 0);

	/** Load game state from the given slot. Returns true if the save existed. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Save")
	bool LoadGame(const FString& SlotName = TEXT("Default"), int32 UserIndex = 0);

	/** Delete a save slot. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Save")
	bool DeleteSave(const FString& SlotName = TEXT("Default"), int32 UserIndex = 0);

	/** Check if a save exists. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Save")
	bool DoesSaveExist(const FString& SlotName = TEXT("Default"), int32 UserIndex = 0) const;

	/** Direct access to the current save data (may be nullptr if nothing loaded). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Save")
	UAnansiSaveGame* GetCurrentSaveData() const { return CurrentSaveData; }

	// -----------------------------------------------------------------------
	// Global audio settings
	// -----------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Anansi|Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Anansi|Audio")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintCallable, Category = "Anansi|Audio")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Anansi|Audio")
	float GetMusicVolume() const { return MusicVolume; }

	UFUNCTION(BlueprintCallable, Category = "Anansi|Audio")
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Anansi|Audio")
	float GetSFXVolume() const { return SFXVolume; }

	// -----------------------------------------------------------------------
	// Persistent data helpers
	// -----------------------------------------------------------------------

	/** Total play-time across all sessions, accumulated here and saved. */
	UPROPERTY(BlueprintReadWrite, Category = "Anansi|Stats")
	float TotalPlayTimeSeconds = 0.0f;

	/** Name of the current level / region the player is in. */
	UPROPERTY(BlueprintReadWrite, Category = "Anansi|World")
	FName CurrentRegionName;

protected:
	UPROPERTY()
	TObjectPtr<UAnansiSaveGame> CurrentSaveData;

	UPROPERTY(Config, BlueprintReadOnly, Category = "Anansi|Audio")
	float MasterVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadOnly, Category = "Anansi|Audio")
	float MusicVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadOnly, Category = "Anansi|Audio")
	float SFXVolume = 1.0f;
};
