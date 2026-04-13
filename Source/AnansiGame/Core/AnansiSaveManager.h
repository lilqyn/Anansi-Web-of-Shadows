// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AnansiSaveManager.generated.h"

/**
 * Serializable player progress data.
 */
USTRUCT(BlueprintType)
struct FPlayerSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) float Health = 100.0f;
	UPROPERTY(BlueprintReadWrite) float WebEnergy = 100.0f;
	UPROPERTY(BlueprintReadWrite) FVector PlayerLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite) FRotator PlayerRotation = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadWrite) FString CurrentMapName;
	UPROPERTY(BlueprintReadWrite) int32 TotalKills = 0;
	UPROPERTY(BlueprintReadWrite) int32 FragmentsCollected = 0;
	UPROPERTY(BlueprintReadWrite) TArray<FName> CollectedFragmentIDs;
	UPROPERTY(BlueprintReadWrite) TArray<FName> CompletedConversations;
	UPROPERTY(BlueprintReadWrite) int32 DifficultyLevel = 1;
	UPROPERTY(BlueprintReadWrite) float PlayTimeSeconds = 0.0f;
	UPROPERTY(BlueprintReadWrite) FString SaveTimestamp;
};

/**
 * UAnansiSaveManager
 *
 * Handles saving and loading player progress to/from disk.
 * Uses JSON serialization to a file in the Saved directory.
 */
UCLASS()
class ANANSIGAME_API UAnansiSaveManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Save current game state to a slot. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Save")
	bool SaveGame(int32 SlotIndex = 0);

	/** Load game state from a slot. Returns true if save exists. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Save")
	bool LoadGame(int32 SlotIndex = 0);

	/** Check if a save exists in a slot. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Save")
	bool DoesSaveExist(int32 SlotIndex = 0) const;

	/** Delete a save slot. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Save")
	void DeleteSave(int32 SlotIndex = 0);

	/** Get the last loaded/saved data. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Save")
	const FPlayerSaveData& GetSaveData() const { return CurrentData; }

	/** Get number of available save slots. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Save")
	int32 GetMaxSlots() const { return 3; }

private:
	FPlayerSaveData CurrentData;

	FString GetSaveFilePath(int32 SlotIndex) const;
	void GatherCurrentState(FPlayerSaveData& OutData);
	void ApplyLoadedState(const FPlayerSaveData& Data);
};
