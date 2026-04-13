// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SoundManager.generated.h"

/**
 * USoundManager
 *
 * World subsystem that plays sound effects for game events.
 * Auto-loads sound assets from known Content paths.
 * Falls back to silent if assets don't exist yet.
 *
 * Import .wav files into these paths and they'll play automatically:
 *   /Game/Audio/SFX/Combat/Hit.Hit
 *   /Game/Audio/SFX/Combat/Whoosh.Whoosh
 *   /Game/Audio/SFX/Combat/Parry.Parry
 *   /Game/Audio/SFX/Combat/Death.Death
 *   /Game/Audio/SFX/Traversal/Jump.Jump
 *   /Game/Audio/SFX/Traversal/Land.Land
 *   /Game/Audio/SFX/Traversal/WebShoot.WebShoot
 *   /Game/Audio/SFX/UI/Pickup.Pickup
 *   /Game/Audio/SFX/UI/Checkpoint.Checkpoint
 */
UCLASS()
class ANANSIGAME_API USoundManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Anansi|Sound")
	void PlayHitSound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Sound")
	void PlayWhooshSound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Sound")
	void PlayParrySound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Sound")
	void PlayDeathSound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Sound")
	void PlayJumpSound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Sound")
	void PlayLandSound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Sound")
	void PlayWebShootSound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Sound")
	void PlayPickupSound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Sound")
	void PlayCheckpointSound(FVector Location);

private:
	void PlaySoundAt(USoundBase* Sound, FVector Location);
	USoundBase* TryLoadSound(const TCHAR* Path);

	UPROPERTY() TObjectPtr<USoundBase> SFX_Hit;
	UPROPERTY() TObjectPtr<USoundBase> SFX_Whoosh;
	UPROPERTY() TObjectPtr<USoundBase> SFX_Parry;
	UPROPERTY() TObjectPtr<USoundBase> SFX_Death;
	UPROPERTY() TObjectPtr<USoundBase> SFX_Jump;
	UPROPERTY() TObjectPtr<USoundBase> SFX_Land;
	UPROPERTY() TObjectPtr<USoundBase> SFX_WebShoot;
	UPROPERTY() TObjectPtr<USoundBase> SFX_Pickup;
	UPROPERTY() TObjectPtr<USoundBase> SFX_Checkpoint;

	int32 LoadedCount = 0;
};
