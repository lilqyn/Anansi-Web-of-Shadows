// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Abilities/GA_TrickMirror.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGA_TrickMirror::UGA_TrickMirror()
{
	EnergyCost = 30.0f;
	CooldownDuration = 15.0f;
}

bool UGA_TrickMirror::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	// Allow re-activation to detonate an existing decoy without energy cost check
	if (bDecoyIsActive)
	{
		return true;
	}

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_TrickMirror::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Re-activation: detonate existing decoy
	if (bDecoyIsActive && SpawnedDecoy)
	{
		DetonateDecoy();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// First activation: spawn decoy
	SpawnDecoy();

	if (!SpawnedDecoy)
	{
		UE_LOG(LogAnansi, Warning, TEXT("TrickMirror — failed to spawn decoy"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bDecoyIsActive = true;

	// Schedule auto-expiration
	if (const ACharacter* OwnerChar = GetOwnerCharacter())
	{
		OwnerChar->GetWorld()->GetTimerManager().SetTimer(
			DecoyTimerHandle, this, &UGA_TrickMirror::OnDecoyExpired, DecoyLifespan, false);
	}

	// Do NOT end the ability yet — it stays active until detonation or expiration
}

void UGA_TrickMirror::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (const ACharacter* OwnerChar = GetOwnerCharacter())
	{
		OwnerChar->GetWorld()->GetTimerManager().ClearTimer(DecoyTimerHandle);
	}

	if (SpawnedDecoy)
	{
		SpawnedDecoy->Destroy();
		SpawnedDecoy = nullptr;
	}

	bDecoyIsActive = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------

void UGA_TrickMirror::SpawnDecoy()
{
	const ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar || !DecoyClass)
	{
		return;
	}

	UWorld* World = OwnerChar->GetWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = const_cast<ACharacter*>(OwnerChar);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	SpawnedDecoy = World->SpawnActor<AActor>(
		DecoyClass,
		OwnerChar->GetActorLocation(),
		OwnerChar->GetActorRotation(),
		SpawnParams);

	if (SpawnedDecoy)
	{
		// Tag the decoy so enemies can target it
		SpawnedDecoy->Tags.Add(FName("Decoy"));
		SpawnedDecoy->Tags.Add(FName("Enemy")); // Makes enemies treat it as a valid target
		SpawnedDecoy->SetLifeSpan(DecoyLifespan + 1.0f); // Safety net

		UE_LOG(LogAnansi, Log, TEXT("TrickMirror — spawned decoy at %s"),
			*OwnerChar->GetActorLocation().ToString());
	}
}

void UGA_TrickMirror::DetonateDecoy()
{
	if (!SpawnedDecoy)
	{
		return;
	}

	UWorld* World = SpawnedDecoy->GetWorld();
	const FVector ExplosionLocation = SpawnedDecoy->GetActorLocation();

	// Spawn VFX
	if (ExplosionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, ExplosionVFX, ExplosionLocation, FRotator::ZeroRotator,
			FVector::OneVector, true, true, ENCPoolMethod::AutoRelease);
	}

	// Play sound
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(World, ExplosionSound, ExplosionLocation);
	}

	// Apply radial damage
	const ACharacter* OwnerChar = GetOwnerCharacter();
	AController* InstigatorController = OwnerChar ? OwnerChar->GetController() : nullptr;

	UGameplayStatics::ApplyRadialDamage(
		World,
		ExplosionDamage,
		ExplosionLocation,
		ExplosionRadius,
		UDamageType::StaticClass(),
		TArray<AActor*>{SpawnedDecoy, const_cast<ACharacter*>(OwnerChar)}, // Ignore decoy and player
		const_cast<ACharacter*>(OwnerChar),
		InstigatorController,
		true, // Do full damage
		ECC_Visibility
	);

	UE_LOG(LogAnansi, Log, TEXT("TrickMirror — detonated decoy (%.0f damage, %.0f radius)"),
		ExplosionDamage, ExplosionRadius);

	SpawnedDecoy->Destroy();
	SpawnedDecoy = nullptr;
	bDecoyIsActive = false;
}

void UGA_TrickMirror::OnDecoyExpired()
{
	DetonateDecoy();

	if (IsActive())
	{
		K2_EndAbility();
	}
}
