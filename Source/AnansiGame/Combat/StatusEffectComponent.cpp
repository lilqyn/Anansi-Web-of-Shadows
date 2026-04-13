// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Combat/StatusEffectComponent.h"
#include "AnansiGame.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

UStatusEffectComponent::UStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UStatusEffectComponent::ApplyEffect(EStatusEffectType Type, float Duration, float DamagePerTick)
{
	// Refresh existing effect of same type
	for (FStatusEffect& Existing : ActiveEffects)
	{
		if (Existing.Type == Type)
		{
			Existing.TimeRemaining = FMath::Max(Existing.TimeRemaining, Duration);
			return;
		}
	}

	FStatusEffect NewEffect;
	NewEffect.Type = Type;
	NewEffect.Duration = Duration;
	NewEffect.TimeRemaining = Duration;
	NewEffect.DamagePerTick = DamagePerTick;
	NewEffect.TickInterval = (Type == EStatusEffectType::Burn) ? 0.3f : 0.5f;
	ActiveEffects.Add(NewEffect);

	// Apply slow on poison
	if (Type == EStatusEffectType::Poison || Type == EStatusEffectType::Slow)
	{
		if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
		{
			if (Char->GetCharacterMovement())
			{
				Char->GetCharacterMovement()->MaxWalkSpeed *= 0.6f;
			}
		}
	}

	UE_LOG(LogAnansi, Verbose, TEXT("StatusEffect: Applied %d to %s (%.1fs)"),
		static_cast<int32>(Type), *GetOwner()->GetName(), Duration);
}

void UStatusEffectComponent::ClearAllEffects()
{
	// Restore speed if poison was active
	if (HasEffect(EStatusEffectType::Poison) || HasEffect(EStatusEffectType::Slow))
	{
		if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
		{
			if (Char->GetCharacterMovement())
			{
				Char->GetCharacterMovement()->MaxWalkSpeed /= 0.6f;
			}
		}
	}
	ActiveEffects.Empty();
}

bool UStatusEffectComponent::HasEffect(EStatusEffectType Type) const
{
	for (const FStatusEffect& Effect : ActiveEffects)
	{
		if (Effect.Type == Type) return true;
	}
	return false;
}

void UStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		FStatusEffect& Effect = ActiveEffects[i];
		Effect.TimeRemaining -= DeltaTime;

		if (Effect.TimeRemaining <= 0.0f)
		{
			// Remove expired effect — restore speed if needed
			if (Effect.Type == EStatusEffectType::Poison || Effect.Type == EStatusEffectType::Slow)
			{
				if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
				{
					if (Char->GetCharacterMovement())
					{
						Char->GetCharacterMovement()->MaxWalkSpeed /= 0.6f;
					}
				}
			}
			ActiveEffects.RemoveAt(i);
			continue;
		}

		ProcessEffect(Effect, DeltaTime);
	}
}

void UStatusEffectComponent::ProcessEffect(FStatusEffect& Effect, float DeltaTime)
{
	Effect.NextTickTime -= DeltaTime;
	if (Effect.NextTickTime > 0.0f) return;

	Effect.NextTickTime = Effect.TickInterval;

	// Apply damage
	if (AActor* Owner = GetOwner())
	{
		FDamageEvent DamageEvent;
		Owner->TakeDamage(Effect.DamagePerTick, DamageEvent, nullptr, nullptr);

#if ENABLE_DRAW_DEBUG
		const FColor Color = (Effect.Type == EStatusEffectType::Burn)  ? FColor::Red :
		                     (Effect.Type == EStatusEffectType::Poison) ? FColor::Green :
		                     (Effect.Type == EStatusEffectType::Bleed)  ? FColor(150, 0, 0) :
		                                                                   FColor::Cyan;
		DrawDebugPoint(GetWorld(), Owner->GetActorLocation() + FVector(0, 0, 90), 20.0f, Color, false, 0.3f);
#endif
	}
}
