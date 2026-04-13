// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/AnansiSaveManager.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "Core/CombatStats.h"
#include "Core/DifficultySettings.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UAnansiSaveManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

FString UAnansiSaveManager::GetSaveFilePath(int32 SlotIndex) const
{
	return FPaths::ProjectSavedDir() / FString::Printf(TEXT("SaveSlot_%d.json"), SlotIndex);
}

bool UAnansiSaveManager::SaveGame(int32 SlotIndex)
{
	GatherCurrentState(CurrentData);

	// Serialize to JSON
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
	Json->SetNumberField(TEXT("Health"), CurrentData.Health);
	Json->SetNumberField(TEXT("WebEnergy"), CurrentData.WebEnergy);
	Json->SetNumberField(TEXT("PosX"), CurrentData.PlayerLocation.X);
	Json->SetNumberField(TEXT("PosY"), CurrentData.PlayerLocation.Y);
	Json->SetNumberField(TEXT("PosZ"), CurrentData.PlayerLocation.Z);
	Json->SetNumberField(TEXT("RotYaw"), CurrentData.PlayerRotation.Yaw);
	Json->SetStringField(TEXT("MapName"), CurrentData.CurrentMapName);
	Json->SetNumberField(TEXT("TotalKills"), CurrentData.TotalKills);
	Json->SetNumberField(TEXT("FragmentsCollected"), CurrentData.FragmentsCollected);
	Json->SetNumberField(TEXT("Difficulty"), CurrentData.DifficultyLevel);
	Json->SetNumberField(TEXT("PlayTime"), CurrentData.PlayTimeSeconds);
	Json->SetStringField(TEXT("Timestamp"), FDateTime::Now().ToString());

	TArray<TSharedPtr<FJsonValue>> FragArray;
	for (const FName& ID : CurrentData.CollectedFragmentIDs)
	{
		FragArray.Add(MakeShareable(new FJsonValueString(ID.ToString())));
	}
	Json->SetArrayField(TEXT("Fragments"), FragArray);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	const FString Path = GetSaveFilePath(SlotIndex);
	if (FFileHelper::SaveStringToFile(JsonString, *Path))
	{
		UE_LOG(LogAnansi, Log, TEXT("Game saved to slot %d (%s)"), SlotIndex, *Path);
		return true;
	}

	UE_LOG(LogAnansi, Warning, TEXT("Failed to save game to slot %d"), SlotIndex);
	return false;
}

bool UAnansiSaveManager::LoadGame(int32 SlotIndex)
{
	const FString Path = GetSaveFilePath(SlotIndex);

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *Path))
	{
		UE_LOG(LogAnansi, Warning, TEXT("No save found in slot %d"), SlotIndex);
		return false;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogAnansi, Warning, TEXT("Failed to parse save file in slot %d"), SlotIndex);
		return false;
	}

	CurrentData.Health = Json->GetNumberField(TEXT("Health"));
	CurrentData.WebEnergy = Json->GetNumberField(TEXT("WebEnergy"));
	CurrentData.PlayerLocation.X = Json->GetNumberField(TEXT("PosX"));
	CurrentData.PlayerLocation.Y = Json->GetNumberField(TEXT("PosY"));
	CurrentData.PlayerLocation.Z = Json->GetNumberField(TEXT("PosZ"));
	CurrentData.PlayerRotation.Yaw = Json->GetNumberField(TEXT("RotYaw"));
	CurrentData.CurrentMapName = Json->GetStringField(TEXT("MapName"));
	CurrentData.TotalKills = Json->GetIntegerField(TEXT("TotalKills"));
	CurrentData.FragmentsCollected = Json->GetIntegerField(TEXT("FragmentsCollected"));
	CurrentData.DifficultyLevel = Json->GetIntegerField(TEXT("Difficulty"));
	CurrentData.PlayTimeSeconds = Json->GetNumberField(TEXT("PlayTime"));
	CurrentData.SaveTimestamp = Json->GetStringField(TEXT("Timestamp"));

	CurrentData.CollectedFragmentIDs.Empty();
	const TArray<TSharedPtr<FJsonValue>>* FragArray;
	if (Json->TryGetArrayField(TEXT("Fragments"), FragArray))
	{
		for (const auto& Val : *FragArray)
		{
			CurrentData.CollectedFragmentIDs.Add(FName(*Val->AsString()));
		}
	}

	ApplyLoadedState(CurrentData);

	UE_LOG(LogAnansi, Log, TEXT("Game loaded from slot %d (Kills: %d, Fragments: %d, PlayTime: %.0fs)"),
		SlotIndex, CurrentData.TotalKills, CurrentData.FragmentsCollected, CurrentData.PlayTimeSeconds);

	return true;
}

bool UAnansiSaveManager::DoesSaveExist(int32 SlotIndex) const
{
	return FPaths::FileExists(GetSaveFilePath(SlotIndex));
}

void UAnansiSaveManager::DeleteSave(int32 SlotIndex)
{
	const FString Path = GetSaveFilePath(SlotIndex);
	IFileManager::Get().Delete(*Path);
	UE_LOG(LogAnansi, Log, TEXT("Deleted save slot %d"), SlotIndex);
}

void UAnansiSaveManager::GatherCurrentState(FPlayerSaveData& OutData)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Player state
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(Player))
	{
		OutData.Health = Anansi->GetCurrentHealth();
		OutData.WebEnergy = Anansi->GetCurrentWebEnergy();
		OutData.PlayerLocation = Anansi->GetActorLocation();
		OutData.PlayerRotation = Anansi->GetActorRotation();
	}

	OutData.CurrentMapName = World->GetMapName();
	OutData.PlayTimeSeconds = World->GetTimeSeconds();

	// Combat stats
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UCombatStatsSubsystem* Stats = GI->GetSubsystem<UCombatStatsSubsystem>())
		{
			OutData.TotalKills = Stats->GetTotalKills();
		}
		if (UDifficultySubsystem* Diff = GI->GetSubsystem<UDifficultySubsystem>())
		{
			OutData.DifficultyLevel = static_cast<int32>(Diff->GetDifficulty());
		}
	}
}

void UAnansiSaveManager::ApplyLoadedState(const FPlayerSaveData& Data)
{
	UWorld* World = GetWorld();
	if (!World) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(Player))
	{
		Anansi->ApplyHealing(Data.Health);
		Anansi->RestoreWebEnergy(Data.WebEnergy);
		Anansi->SetActorLocation(Data.PlayerLocation);
		Anansi->SetActorRotation(Data.PlayerRotation);
	}

	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UDifficultySubsystem* Diff = GI->GetSubsystem<UDifficultySubsystem>())
		{
			Diff->SetDifficulty(static_cast<EDifficulty>(FMath::Clamp(Data.DifficultyLevel, 0, 2)));
		}
	}
}
