// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/DifficultySettings.h"
#include "AnansiGame.h"

void UDifficultySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SetDifficulty(EDifficulty::Normal);
}

void UDifficultySubsystem::SetDifficulty(EDifficulty NewDifficulty)
{
	CurrentDifficulty = NewDifficulty;

	switch (CurrentDifficulty)
	{
	case EDifficulty::Easy:
		PlayerDamageMult = 1.5f;
		EnemyDamageMult = 0.5f;
		DetectionSpeedMult = 0.6f;
		RegenMult = 2.0f;
		break;

	case EDifficulty::Normal:
		PlayerDamageMult = 1.0f;
		EnemyDamageMult = 1.0f;
		DetectionSpeedMult = 1.0f;
		RegenMult = 1.0f;
		break;

	case EDifficulty::Hard:
		PlayerDamageMult = 0.7f;
		EnemyDamageMult = 1.8f;
		DetectionSpeedMult = 1.5f;
		RegenMult = 0.5f;
		break;
	}

	UE_LOG(LogAnansi, Log, TEXT("Difficulty set to: %s (Player dmg x%.1f, Enemy dmg x%.1f)"),
		*GetDifficultyName(), PlayerDamageMult, EnemyDamageMult);
}

FString UDifficultySubsystem::GetDifficultyName() const
{
	switch (CurrentDifficulty)
	{
	case EDifficulty::Easy:   return TEXT("Easy");
	case EDifficulty::Normal: return TEXT("Normal");
	case EDifficulty::Hard:   return TEXT("Hard");
	default:                  return TEXT("Unknown");
	}
}
