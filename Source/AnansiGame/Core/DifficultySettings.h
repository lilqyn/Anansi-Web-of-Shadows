// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DifficultySettings.generated.h"

UENUM(BlueprintType)
enum class EDifficulty : uint8
{
	Easy	UMETA(DisplayName = "Easy"),
	Normal	UMETA(DisplayName = "Normal"),
	Hard	UMETA(DisplayName = "Hard")
};

/**
 * UDifficultySubsystem
 *
 * Manages game difficulty. Scales enemy damage, player damage,
 * detection speed, and resource regeneration.
 */
UCLASS()
class ANANSIGAME_API UDifficultySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Anansi|Difficulty")
	void SetDifficulty(EDifficulty NewDifficulty);

	UFUNCTION(BlueprintPure, Category = "Anansi|Difficulty")
	EDifficulty GetDifficulty() const { return CurrentDifficulty; }

	/** Multiplier applied to damage the player deals. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Difficulty")
	float GetPlayerDamageMultiplier() const { return PlayerDamageMult; }

	/** Multiplier applied to damage enemies deal. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Difficulty")
	float GetEnemyDamageMultiplier() const { return EnemyDamageMult; }

	/** Multiplier for enemy detection speed (stealth). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Difficulty")
	float GetDetectionSpeedMultiplier() const { return DetectionSpeedMult; }

	/** Multiplier for resource regeneration rate. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Difficulty")
	float GetRegenMultiplier() const { return RegenMult; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Difficulty")
	FString GetDifficultyName() const;

private:
	EDifficulty CurrentDifficulty = EDifficulty::Normal;
	float PlayerDamageMult = 1.0f;
	float EnemyDamageMult = 1.0f;
	float DetectionSpeedMult = 1.0f;
	float RegenMult = 1.0f;
};
