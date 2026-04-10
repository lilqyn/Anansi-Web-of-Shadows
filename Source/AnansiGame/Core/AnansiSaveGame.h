// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AnansiSaveGame.generated.h"

/**
 * Serialised representation of a single ability upgrade the player has purchased.
 */
USTRUCT(BlueprintType)
struct FAbilityUpgradeSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
	FName AbilityID;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
	int32 UpgradeLevel = 0;
};

/**
 * Serialised inventory item.
 */
USTRUCT(BlueprintType)
struct FInventoryItemSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
	FName ItemID;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save")
	int32 Quantity = 1;
};

/**
 * UAnansiSaveGame
 *
 * Concrete save-game object that captures all persistent player data:
 *  - Player stats (health, energy, stamina)
 *  - Inventory contents
 *  - Story progress and collected fragments
 *  - World-state flags
 *  - Ability upgrade tree state
 *  - Player transform so they can resume at the same spot
 */
UCLASS()
class ANANSIGAME_API UAnansiSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UAnansiSaveGame();

	// -----------------------------------------------------------------------
	// Meta
	// -----------------------------------------------------------------------

	/** Incremented on each save — used for version migration if the schema changes. */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Meta")
	int32 SaveVersion = 1;

	/** Timestamp of the save. */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Meta")
	FDateTime SaveTimestamp;

	/** Total play-time in seconds up to the point of saving. */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Meta")
	float TotalPlayTimeSeconds = 0.0f;

	// -----------------------------------------------------------------------
	// Player stats
	// -----------------------------------------------------------------------

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Player")
	float CurrentHealth = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Player")
	float MaxHealth = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Player")
	float CurrentWebEnergy = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Player")
	float MaxWebEnergy = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Player")
	float CurrentStamina = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Player")
	float MaxStamina = 100.0f;

	/** Player world transform so they resume where they left off. */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Player")
	FTransform PlayerTransform;

	/** Name of the level/map the player was on. */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Player")
	FName LastMapName;

	// -----------------------------------------------------------------------
	// Inventory
	// -----------------------------------------------------------------------

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Inventory")
	TArray<FInventoryItemSaveData> InventoryItems;

	// -----------------------------------------------------------------------
	// Story progress
	// -----------------------------------------------------------------------

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Story")
	int32 MainQuestIndex = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Story")
	TArray<FName> CollectedFragmentIDs;

	// -----------------------------------------------------------------------
	// World state
	// -----------------------------------------------------------------------

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|World")
	TMap<FName, bool> WorldFlags;

	// -----------------------------------------------------------------------
	// Ability upgrades
	// -----------------------------------------------------------------------

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Abilities")
	TArray<FAbilityUpgradeSaveData> AbilityUpgrades;

	/** Unspent upgrade currency. */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save|Abilities")
	int32 UpgradePoints = 0;
};
