// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Abilities/GA_BorrowedVoice.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AIPerceptionSystem.h"

UGA_BorrowedVoice::UGA_BorrowedVoice()
{
	EnergyCost = 15.0f;
	CooldownDuration = 8.0f;
}

void UGA_BorrowedVoice::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Determine target location from camera aim
	FHitResult Hit;
	FVector TargetLocation;

	if (PerformCameraLineTrace(MaxPlacementRange, Hit))
	{
		TargetLocation = Hit.ImpactPoint;
	}
	else
	{
		// Place at max range in the aimed direction
		const APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
		if (PC && PC->PlayerCameraManager)
		{
			TargetLocation = PC->PlayerCameraManager->GetCameraLocation()
				+ PC->PlayerCameraManager->GetCameraRotation().Vector() * MaxPlacementRange;
		}
		else
		{
			TargetLocation = OwnerChar->GetActorLocation()
				+ OwnerChar->GetActorForwardVector() * MaxPlacementRange;
		}
	}

	UWorld* World = OwnerChar->GetWorld();
	PlaceSoundSource(TargetLocation, World);
	AlertNearbyAI(TargetLocation, World);

	// Schedule cleanup
	World->GetTimerManager().SetTimer(SoundTimerHandle, this,
		&UGA_BorrowedVoice::OnSoundExpired, SoundDuration, false);
}

void UGA_BorrowedVoice::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	OnSoundExpired();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------

void UGA_BorrowedVoice::PlaceSoundSource(const FVector& Location, UWorld* World)
{
	if (!World)
	{
		return;
	}

	// Spawn audio
	if (VoiceSound)
	{
		ActiveAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
			World, VoiceSound, Location, FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f,
			nullptr, nullptr, true);
	}

	// Spawn visual indicator
	if (SoundIndicatorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnedIndicator = World->SpawnActor<AActor>(SoundIndicatorClass, Location,
			FRotator::ZeroRotator, SpawnParams);
	}

	UE_LOG(LogAnansi, Log, TEXT("BorrowedVoice — placed sound source at %s"), *Location.ToString());
}

void UGA_BorrowedVoice::AlertNearbyAI(const FVector& Location, UWorld* World)
{
	if (!World)
	{
		return;
	}

	// Report a noise event to the AI perception system
	const ACharacter* OwnerChar = GetOwnerCharacter();
	AActor* Instigator = const_cast<ACharacter*>(OwnerChar);

	UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(World);
	if (PerceptionSystem)
	{
		FAINoiseEvent NoiseEvent;
		NoiseEvent.NoiseLocation = Location;
		NoiseEvent.Loudness = 1.0f;
		NoiseEvent.MaxRange = SoundAttractionRadius;
		NoiseEvent.Instigator = Instigator;
		NoiseEvent.Tag = FName("BorrowedVoice");
		PerceptionSystem->OnEvent(NoiseEvent);

		UE_LOG(LogAnansi, Log, TEXT("BorrowedVoice — reported noise event (radius %.0f)"), SoundAttractionRadius);
	}
}

void UGA_BorrowedVoice::OnSoundExpired()
{
	if (ActiveAudioComponent)
	{
		ActiveAudioComponent->Stop();
		ActiveAudioComponent = nullptr;
	}

	if (SpawnedIndicator)
	{
		SpawnedIndicator->Destroy();
		SpawnedIndicator = nullptr;
	}

	if (const ACharacter* OwnerChar = GetOwnerCharacter())
	{
		OwnerChar->GetWorld()->GetTimerManager().ClearTimer(SoundTimerHandle);
	}
}
