// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Abilities/AnansiGameplayAbility.h"
#include "Abilities/AnansiAbilitySystemComponent.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UAnansiGameplayAbility::UAnansiGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

// ---------------------------------------------------------------------------
// Activation guards
// ---------------------------------------------------------------------------

bool UAnansiGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check blocking tags (stunned, dead, etc.)
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (BlockingTags.Num() > 0 && ASC->HasAnyMatchingGameplayTags(BlockingTags))
		{
			UE_LOG(LogAnansi, Verbose, TEXT("%s blocked by tags"), *GetName());
			return false;
		}
	}

	// Check energy cost
	const UAnansiAbilitySystemComponent* AnansiASC = GetAnansiAbilitySystemComponent();
	if (AnansiASC && EnergyCost > 0.0f)
	{
		if (AnansiASC->GetCurrentWebEnergy() < EnergyCost)
		{
			UE_LOG(LogAnansi, Verbose, TEXT("%s — insufficient web energy (need %.1f, have %.1f)"),
				*GetName(), EnergyCost, AnansiASC->GetCurrentWebEnergy());
			return false;
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// Activation / End
// ---------------------------------------------------------------------------

void UAnansiGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Spend energy
	UAnansiAbilitySystemComponent* AnansiASC = GetAnansiAbilitySystemComponent();
	if (AnansiASC && EnergyCost > 0.0f)
	{
		if (!AnansiASC->SpendWebEnergy(EnergyCost))
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
	}

	// Apply cooldown
	if (CooldownDuration > 0.0f)
	{
		ApplyCooldownEffect();
	}

	// Delegate to subclass
	OnAbilityActivated(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAnansiGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAnansiGameplayAbility::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Default implementation — subclasses override this.
}

// ---------------------------------------------------------------------------
// Cooldown
// ---------------------------------------------------------------------------

void UAnansiGameplayAbility::ApplyCooldownEffect()
{
	if (!GetAbilitySystemComponentFromActorInfo())
	{
		return;
	}

	const FGameplayTagContainer* CooldownTags = GetCooldownTags();
	if (CooldownTags && CooldownTags->Num() > 0)
	{
		// Use parent class cooldown mechanism if cooldown tags are configured
		CommitAbilityCooldown(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(),
			GetCurrentActivationInfo(), false);
	}
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

UAnansiAbilitySystemComponent* UAnansiGameplayAbility::GetAnansiAbilitySystemComponent() const
{
	return Cast<UAnansiAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}

ACharacter* UAnansiGameplayAbility::GetOwnerCharacter() const
{
	if (const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo())
	{
		return Cast<ACharacter>(Info->AvatarActor.Get());
	}
	return nullptr;
}

bool UAnansiGameplayAbility::PerformCameraLineTrace(float TraceDistance, FHitResult& OutHit) const
{
	const ACharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return false;
	}

	const APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC || !PC->PlayerCameraManager)
	{
		return false;
	}

	const FVector Start = PC->PlayerCameraManager->GetCameraLocation();
	const FVector End = Start + PC->PlayerCameraManager->GetCameraRotation().Vector() * TraceDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);
	Params.bTraceComplex = true;

	return Character->GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}
