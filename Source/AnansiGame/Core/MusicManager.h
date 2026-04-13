// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "MusicManager.generated.h"

class USoundBase;
class UAudioComponent;

UENUM(BlueprintType)
enum class EMusicState : uint8
{
	Silent		UMETA(DisplayName = "Silent"),
	Exploration	UMETA(DisplayName = "Exploration"),
	Combat		UMETA(DisplayName = "Combat"),
	Boss		UMETA(DisplayName = "Boss"),
	Stealth		UMETA(DisplayName = "Stealth")
};

/**
 * UMusicManager
 *
 * World subsystem that plays music tracks based on gameplay state.
 * Automatically switches between exploration and combat music based on
 * nearby enemy awareness. Fades between tracks.
 *
 * Import .wav/.uasset tracks to these paths for auto-play:
 *   /Game/Audio/Music/Exploration.Exploration
 *   /Game/Audio/Music/Combat.Combat
 *   /Game/Audio/Music/Boss.Boss
 *   /Game/Audio/Music/Stealth.Stealth
 */
UCLASS()
class ANANSIGAME_API UMusicManager : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	}

	UFUNCTION(BlueprintCallable, Category = "Anansi|Music")
	void SetMusicState(EMusicState NewState);

	UFUNCTION(BlueprintPure, Category = "Anansi|Music")
	EMusicState GetMusicState() const { return CurrentState; }

private:
	EMusicState CurrentState = EMusicState::Silent;
	float StateCheckTimer = 0.0f;

	UPROPERTY() TObjectPtr<USoundBase> Track_Exploration;
	UPROPERTY() TObjectPtr<USoundBase> Track_Combat;
	UPROPERTY() TObjectPtr<USoundBase> Track_Boss;
	UPROPERTY() TObjectPtr<USoundBase> Track_Stealth;

	UPROPERTY() TObjectPtr<UAudioComponent> ActiveAudioComp;

	void CheckGameplayState();
	USoundBase* GetTrackForState(EMusicState State) const;
};
