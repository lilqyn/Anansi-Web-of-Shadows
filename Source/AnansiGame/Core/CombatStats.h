// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CombatStats.generated.h"

/**
 * UCombatStatsSubsystem
 *
 * Tracks combat performance across encounters. Persists for the game session.
 * Feeds into end-of-encounter rating and HUD display.
 */
UCLASS()
class ANANSIGAME_API UCombatStatsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Record a hit dealt by the player. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void RecordHit(float Damage, bool bIsCritical = false);

	/** Record the player taking damage. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void RecordDamageTaken(float Damage);

	/** Record an enemy killed. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void RecordKill();

	/** Record a successful parry. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void RecordParry();

	/** Record a successful dodge. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void RecordDodge();

	/** Record a player death. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void RecordDeath();

	/** Reset all stats (new encounter). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void ResetStats();

	/** Get a letter grade (S/A/B/C/D) based on performance. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	FString GetPerformanceGrade() const;

	/** Get a formatted summary string. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	FString GetStatsSummary() const;

	// -- Accessors -----------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	int32 GetTotalHits() const { return TotalHits; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	float GetTotalDamageDealt() const { return TotalDamageDealt; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	int32 GetTotalKills() const { return TotalKills; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	int32 GetMaxCombo() const { return MaxCombo; }

	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void SetMaxCombo(int32 Combo) { MaxCombo = FMath::Max(MaxCombo, Combo); }

private:
	int32 TotalHits = 0;
	int32 CriticalHits = 0;
	float TotalDamageDealt = 0.0f;
	float TotalDamageTaken = 0.0f;
	int32 TotalKills = 0;
	int32 TotalParries = 0;
	int32 TotalDodges = 0;
	int32 TotalDeaths = 0;
	int32 MaxCombo = 0;
	float EncounterStartTime = 0.0f;
};
