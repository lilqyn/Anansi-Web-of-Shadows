// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Abilities/AnansiAbilitySystemComponent.h"
#include "AnansiGame.h"
#include "Abilities/AnansiGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"

UAnansiAbilitySystemComponent::UAnansiAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UAnansiAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentWebEnergy = MaxWebEnergy;
	BroadcastEnergyChange();
}

void UAnansiAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Regenerate web energy after cooldown
	if (RegenCooldownRemaining > 0.0f)
	{
		RegenCooldownRemaining -= DeltaTime;
	}
	else if (CurrentWebEnergy < MaxWebEnergy && WebEnergyRegenRate > 0.0f)
	{
		const float OldEnergy = CurrentWebEnergy;
		CurrentWebEnergy = FMath::Min(CurrentWebEnergy + WebEnergyRegenRate * DeltaTime, MaxWebEnergy);
		if (!FMath::IsNearlyEqual(OldEnergy, CurrentWebEnergy))
		{
			BroadcastEnergyChange();
		}
	}
}

// ---------------------------------------------------------------------------
// Web energy
// ---------------------------------------------------------------------------

bool UAnansiAbilitySystemComponent::SpendWebEnergy(float Amount)
{
	if (Amount <= 0.0f)
	{
		return true;
	}

	if (CurrentWebEnergy < Amount)
	{
		UE_LOG(LogAnansi, Verbose, TEXT("SpendWebEnergy failed — need %.1f, have %.1f"), Amount, CurrentWebEnergy);
		return false;
	}

	CurrentWebEnergy -= Amount;
	RegenCooldownRemaining = RegenCooldownDuration;
	BroadcastEnergyChange();
	return true;
}

void UAnansiAbilitySystemComponent::AddWebEnergy(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	CurrentWebEnergy = FMath::Min(CurrentWebEnergy + Amount, MaxWebEnergy);
	BroadcastEnergyChange();
}

void UAnansiAbilitySystemComponent::SetMaxWebEnergy(float NewMax)
{
	MaxWebEnergy = FMath::Max(1.0f, NewMax);
	CurrentWebEnergy = FMath::Min(CurrentWebEnergy, MaxWebEnergy);
	BroadcastEnergyChange();
}

float UAnansiAbilitySystemComponent::GetWebEnergyPercent() const
{
	return (MaxWebEnergy > 0.0f) ? CurrentWebEnergy / MaxWebEnergy : 0.0f;
}

void UAnansiAbilitySystemComponent::BroadcastEnergyChange()
{
	OnWebEnergyChanged.Broadcast(CurrentWebEnergy, MaxWebEnergy);
}

// ---------------------------------------------------------------------------
// Cooldown helpers
// ---------------------------------------------------------------------------

float UAnansiAbilitySystemComponent::GetCooldownRemainingForAbility(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if (!AbilityClass)
	{
		return 0.0f;
	}

	const UGameplayAbility* AbilityCDO = AbilityClass.GetDefaultObject();
	if (!AbilityCDO)
	{
		return 0.0f;
	}

	const FGameplayTagContainer* CooldownTags = AbilityCDO->GetCooldownTags();
	if (!CooldownTags || CooldownTags->Num() == 0)
	{
		return 0.0f;
	}

	float MaxRemaining = 0.0f;
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
	TArray<TPair<float, float>> DurationsAndRemaining = GetActiveEffectsTimeRemainingAndDuration(Query);
	for (const auto& Pair : DurationsAndRemaining)
	{
		MaxRemaining = FMath::Max(MaxRemaining, Pair.Key);
	}

	return MaxRemaining;
}

bool UAnansiAbilitySystemComponent::IsAbilityOffCooldown(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	return GetCooldownRemainingForAbility(AbilityClass) <= 0.0f;
}

// ---------------------------------------------------------------------------
// Activation
// ---------------------------------------------------------------------------

bool UAnansiAbilitySystemComponent::TryActivateAbilityWithTag(FGameplayTag AbilityTag)
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(AbilityTag);
	return TryActivateAbilitiesByTag(TagContainer);
}

void UAnansiAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle,
	UGameplayAbility* Ability, bool bWasCancelled)
{
	ActiveAbilityCount = FMath::Max(0, ActiveAbilityCount - 1);
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);
}

// ---------------------------------------------------------------------------
// Active ability tracking
// ---------------------------------------------------------------------------

bool UAnansiAbilitySystemComponent::HasActiveAbilityWithTag(FGameplayTag Tag) const
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(Tag);
	TArray<FGameplayAbilitySpec*> MatchingSpecs;
	const_cast<UAnansiAbilitySystemComponent*>(this)->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
		TagContainer, MatchingSpecs);

	for (const FGameplayAbilitySpec* Spec : MatchingSpecs)
	{
		if (Spec && Spec->IsActive())
		{
			return true;
		}
	}

	return false;
}
