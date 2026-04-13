// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/CombatStats.h"
#include "AnansiGame.h"

void UCombatStatsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetStats();
}

void UCombatStatsSubsystem::RecordHit(float Damage, bool bIsCritical)
{
	TotalHits++;
	TotalDamageDealt += Damage;
	if (bIsCritical) CriticalHits++;
}

void UCombatStatsSubsystem::RecordDamageTaken(float Damage)
{
	TotalDamageTaken += Damage;
}

void UCombatStatsSubsystem::RecordKill()
{
	TotalKills++;
}

void UCombatStatsSubsystem::RecordParry()
{
	TotalParries++;
}

void UCombatStatsSubsystem::RecordDodge()
{
	TotalDodges++;
}

void UCombatStatsSubsystem::RecordDeath()
{
	TotalDeaths++;
}

void UCombatStatsSubsystem::ResetStats()
{
	TotalHits = 0;
	CriticalHits = 0;
	TotalDamageDealt = 0.0f;
	TotalDamageTaken = 0.0f;
	TotalKills = 0;
	TotalParries = 0;
	TotalDodges = 0;
	TotalDeaths = 0;
	MaxCombo = 0;
	EncounterStartTime = 0.0f;
}

void UCombatStatsSubsystem::RecordStreakKill()
{
	RecordKill();

	UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;

	if (CurrentTime - KillStreakTimer <= KillStreakWindow)
	{
		KillStreak++;
	}
	else
	{
		KillStreak = 1;
	}

	KillStreakTimer = CurrentTime;

	if (KillStreak >= 3)
	{
		UE_LOG(LogAnansi, Log, TEXT("Kill Streak: %d! (x%.1f damage)"), KillStreak, GetKillStreakMultiplier());
	}
}

float UCombatStatsSubsystem::GetKillStreakMultiplier() const
{
	if (KillStreak < 3) return 1.0f;
	if (KillStreak < 5) return 1.25f;
	if (KillStreak < 8) return 1.5f;
	return 2.0f; // 8+ kills = double damage
}

FString UCombatStatsSubsystem::GetPerformanceGrade() const
{
	// Score based on: kills, combos, parries, damage efficiency
	float Score = 0.0f;

	Score += TotalKills * 100.0f;
	Score += MaxCombo * 10.0f;
	Score += TotalParries * 50.0f;
	Score += TotalDodges * 25.0f;
	Score += CriticalHits * 30.0f;

	// Penalties
	Score -= TotalDeaths * 200.0f;
	Score -= TotalDamageTaken * 2.0f;

	if (Score >= 2000) return TEXT("S");
	if (Score >= 1200) return TEXT("A");
	if (Score >= 600)  return TEXT("B");
	if (Score >= 200)  return TEXT("C");
	return TEXT("D");
}

FString UCombatStatsSubsystem::GetStatsSummary() const
{
	return FString::Printf(
		TEXT("Grade: %s | Kills: %d | Hits: %d | Max Combo: %d | Parries: %d | Dodges: %d | Deaths: %d | Damage Dealt: %.0f | Taken: %.0f"),
		*GetPerformanceGrade(), TotalKills, TotalHits, MaxCombo, TotalParries, TotalDodges,
		TotalDeaths, TotalDamageDealt, TotalDamageTaken);
}
