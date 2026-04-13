// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/AchievementSystem.h"
#include "AnansiGame.h"
#include "UI/AnansiDevHUD.h"
#include "Kismet/GameplayStatics.h"

void UAchievementSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisterAchievements();
}

void UAchievementSystem::RegisterAchievements()
{
	auto Register = [this](const TCHAR* ID, const TCHAR* Title, const TCHAR* Desc)
	{
		FAchievement Ach;
		Ach.ID = FName(ID);
		Ach.Title = Title;
		Ach.Description = Desc;
		Achievements.Add(Ach.ID, Ach);
	};

	Register(TEXT("FirstBlood"),      TEXT("First Blood"),        TEXT("Defeat your first enemy"));
	Register(TEXT("SlayerI"),         TEXT("Slayer I"),           TEXT("Defeat 10 enemies"));
	Register(TEXT("SlayerII"),        TEXT("Slayer II"),          TEXT("Defeat 50 enemies"));
	Register(TEXT("SlayerIII"),       TEXT("Slayer III"),         TEXT("Defeat 100 enemies"));
	Register(TEXT("ComboNovice"),     TEXT("Combo Novice"),       TEXT("Reach a 10-hit combo"));
	Register(TEXT("ComboMaster"),     TEXT("Combo Master"),       TEXT("Reach a 30-hit combo"));
	Register(TEXT("ComboLegend"),     TEXT("Combo Legend"),       TEXT("Reach a 50-hit combo"));
	Register(TEXT("BossSlayer"),      TEXT("Boss Slayer"),        TEXT("Defeat the Captain of the Mask"));
	Register(TEXT("Stealthy"),        TEXT("Stealthy"),           TEXT("Perform a stealth takedown"));
	Register(TEXT("Storyteller"),     TEXT("Storyteller"),        TEXT("Collect your first story fragment"));
	Register(TEXT("LoreKeeper"),      TEXT("Lore Keeper"),        TEXT("Collect 10 story fragments"));
	Register(TEXT("Survivor"),        TEXT("Survivor"),           TEXT("Survive 5 waves of the survival mode"));
	Register(TEXT("Unbreakable"),     TEXT("Unbreakable"),        TEXT("Complete an encounter without taking damage"));
	Register(TEXT("HighSwinger"),     TEXT("High Swinger"),       TEXT("Use web-swing to travel 2000 units"));
	Register(TEXT("Tricky"),          TEXT("Tricky"),             TEXT("Defeat 3 enemies with a single Trick Mirror detonation"));

	UE_LOG(LogAnansi, Log, TEXT("AchievementSystem: %d achievements registered"), Achievements.Num());
}

bool UAchievementSystem::UnlockAchievement(FName ID)
{
	FAchievement* Found = Achievements.Find(ID);
	if (!Found || Found->bUnlocked) return false;

	Found->bUnlocked = true;
	OnAchievementUnlocked.Broadcast(*Found);
	ShowUnlockToast(*Found);

	UE_LOG(LogAnansi, Log, TEXT("Achievement Unlocked: %s — %s"), *Found->Title, *Found->Description);
	return true;
}

bool UAchievementSystem::IsUnlocked(FName ID) const
{
	const FAchievement* Found = Achievements.Find(ID);
	return Found && Found->bUnlocked;
}

int32 UAchievementSystem::GetUnlockedCount() const
{
	int32 Count = 0;
	for (const auto& Pair : Achievements)
	{
		if (Pair.Value.bUnlocked) Count++;
	}
	return Count;
}

TArray<FAchievement> UAchievementSystem::GetAllAchievements() const
{
	TArray<FAchievement> Result;
	for (const auto& Pair : Achievements)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

void UAchievementSystem::CheckKillCount(int32 TotalKills)
{
	if (TotalKills >= 1)   UnlockAchievement(FName("FirstBlood"));
	if (TotalKills >= 10)  UnlockAchievement(FName("SlayerI"));
	if (TotalKills >= 50)  UnlockAchievement(FName("SlayerII"));
	if (TotalKills >= 100) UnlockAchievement(FName("SlayerIII"));
}

void UAchievementSystem::CheckCombo(int32 MaxCombo)
{
	if (MaxCombo >= 10) UnlockAchievement(FName("ComboNovice"));
	if (MaxCombo >= 30) UnlockAchievement(FName("ComboMaster"));
	if (MaxCombo >= 50) UnlockAchievement(FName("ComboLegend"));
}

void UAchievementSystem::ShowUnlockToast(const FAchievement& Achievement)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->ShowToast(FString::Printf(TEXT("ACHIEVEMENT: %s"), *Achievement.Title),
				FColor(255, 200, 50));
			HUD->FlashScreen(FLinearColor(1.0f, 0.8f, 0.2f), 0.3f);
		}
	}
}
