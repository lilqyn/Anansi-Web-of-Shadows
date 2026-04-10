// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Abilities/GA_StoryBind.h"
#include "AnansiGame.h"
#include "Narrative/StoryFragmentSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGA_StoryBind::UGA_StoryBind()
{
	EnergyCost = 35.0f;
	CooldownDuration = 20.0f;
}

bool UGA_StoryBind::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check fragment count
	const UStoryFragmentSystem* FragmentSystem = GetStoryFragmentSystem();
	if (!FragmentSystem || FragmentSystem->GetCollectedFragmentCount() < MinFragmentsRequired)
	{
		UE_LOG(LogAnansi, Verbose, TEXT("StoryBind — insufficient story fragments"));
		return false;
	}

	return true;
}

void UGA_StoryBind::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
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

	// Line trace for target
	FHitResult Hit;
	if (!PerformCameraLineTrace(TargetRange, Hit) || !Hit.GetActor())
	{
		UE_LOG(LogAnansi, Verbose, TEXT("StoryBind — no valid target"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AActor* Target = Hit.GetActor();
	const bool bIsSpirit = Target->ActorHasTag(SpiritTag);
	const bool bIsBoss = Target->ActorHasTag(BossTag);

	if (!bIsSpirit && !bIsBoss)
	{
		UE_LOG(LogAnansi, Verbose, TEXT("StoryBind — target is not a spirit or boss"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Calculate effect duration
	const UStoryFragmentSystem* FragmentSystem = GetStoryFragmentSystem();
	const int32 FragmentCount = FragmentSystem ? FragmentSystem->GetCollectedFragmentCount() : MinFragmentsRequired;

	float Duration;
	if (bIsBoss)
	{
		// Bosses get a fixed vulnerability window, slightly scaled
		const float BonusFactor = FMath::Min(static_cast<float>(FragmentCount) / 10.0f, 2.0f);
		Duration = BossVulnerabilityDuration * BonusFactor;
	}
	else
	{
		Duration = CalculateRootDuration(FragmentCount);
	}

	ApplyBindEffect(Target, Duration);

	// Schedule release
	OwnerChar->GetWorld()->GetTimerManager().SetTimer(
		BindTimerHandle, this, &UGA_StoryBind::ReleaseBind, Duration, false);
}

void UGA_StoryBind::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	ReleaseBind();

	if (const ACharacter* OwnerChar = GetOwnerCharacter())
	{
		OwnerChar->GetWorld()->GetTimerManager().ClearTimer(BindTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------

UStoryFragmentSystem* UGA_StoryBind::GetStoryFragmentSystem() const
{
	const ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar)
	{
		return nullptr;
	}

	const UGameInstance* GI = UGameplayStatics::GetGameInstance(OwnerChar);
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UStoryFragmentSystem>();
}

float UGA_StoryBind::CalculateRootDuration(int32 FragmentCount) const
{
	const int32 ExtraFragments = FMath::Max(0, FragmentCount - MinFragmentsRequired);
	return BaseRootDuration + ExtraFragments * DurationPerExtraFragment;
}

void UGA_StoryBind::ApplyBindEffect(AActor* Target, float Duration)
{
	if (!Target)
	{
		return;
	}

	BoundTarget = Target;

	// Disable movement on the target if it is a character
	if (ACharacter* TargetChar = Cast<ACharacter>(Target))
	{
		if (UCharacterMovementComponent* Movement = TargetChar->GetCharacterMovement())
		{
			Movement->DisableMovement();
		}
	}

	// Spawn golden chains VFX attached to the target
	if (GoldenChainsVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			GoldenChainsVFX,
			Target->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true,
			true,
			ENCPoolMethod::AutoRelease);
	}

	// Play sound
	if (BindSound)
	{
		UGameplayStatics::PlaySoundAtLocation(Target->GetWorld(), BindSound, Target->GetActorLocation());
	}

	// Tag the target as vulnerable (for boss damage multiplier systems)
	Target->Tags.AddUnique(FName("StoryBound"));

	UE_LOG(LogAnansi, Log, TEXT("StoryBind — bound %s for %.1f seconds"), *Target->GetName(), Duration);
}

void UGA_StoryBind::ReleaseBind()
{
	if (!BoundTarget)
	{
		return;
	}

	// Re-enable movement
	if (ACharacter* TargetChar = Cast<ACharacter>(BoundTarget))
	{
		if (UCharacterMovementComponent* Movement = TargetChar->GetCharacterMovement())
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}

	// Remove bound tag
	BoundTarget->Tags.Remove(FName("StoryBound"));

	UE_LOG(LogAnansi, Log, TEXT("StoryBind — released %s"), *BoundTarget->GetName());

	BoundTarget = nullptr;

	if (IsActive())
	{
		K2_EndAbility();
	}
}
