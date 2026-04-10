// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/AnansiGameplayAbility.h"
#include "GA_BorrowedVoice.generated.h"

class UAudioComponent;

/**
 * UGA_BorrowedVoice
 *
 * Anansi imitates a voice to create a distracting sound source at a target
 * location. AI enemies within hearing range investigate the sound, allowing
 * the player to bypass or reposition.
 */
UCLASS()
class ANANSIGAME_API UGA_BorrowedVoice : public UAnansiGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_BorrowedVoice();

protected:
	virtual void OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	// -------------------------------------------------------------------
	// Configuration
	// -------------------------------------------------------------------

	/** Maximum range for placing the sound source. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|BorrowedVoice")
	float MaxPlacementRange = 2000.0f;

	/** Radius in which AI will hear and investigate the sound. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|BorrowedVoice")
	float SoundAttractionRadius = 1200.0f;

	/** How long the decoy sound persists (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|BorrowedVoice", meta = (ClampMin = "1.0"))
	float SoundDuration = 6.0f;

	/** Sound cue played at the target location. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|BorrowedVoice")
	TObjectPtr<USoundBase> VoiceSound;

	/** Visual indicator spawned at the sound source (e.g. echo particle). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|BorrowedVoice")
	TSubclassOf<AActor> SoundIndicatorClass;

private:
	/** Place the sound source at the aimed location. */
	void PlaceSoundSource(const FVector& Location, UWorld* World);

	/** Notify nearby AI enemies about the noise. */
	void AlertNearbyAI(const FVector& Location, UWorld* World);

	/** Clean up the sound source when the effect expires. */
	void OnSoundExpired();

	UPROPERTY()
	TObjectPtr<UAudioComponent> ActiveAudioComponent;

	UPROPERTY()
	TObjectPtr<AActor> SpawnedIndicator;

	FTimerHandle SoundTimerHandle;
};
