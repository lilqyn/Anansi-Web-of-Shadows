// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusEffectComponent.generated.h"

UENUM(BlueprintType)
enum class EStatusEffectType : uint8
{
	Burn	UMETA(DisplayName = "Burn"),
	Poison	UMETA(DisplayName = "Poison"),
	Bleed	UMETA(DisplayName = "Bleed"),
	Slow	UMETA(DisplayName = "Slow")
};

USTRUCT(BlueprintType)
struct FStatusEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) EStatusEffectType Type = EStatusEffectType::Burn;
	UPROPERTY(BlueprintReadOnly) float Duration = 5.0f;
	UPROPERTY(BlueprintReadOnly) float TickInterval = 0.5f;
	UPROPERTY(BlueprintReadOnly) float DamagePerTick = 2.0f;
	UPROPERTY(BlueprintReadOnly) float TimeRemaining = 0.0f;
	UPROPERTY(BlueprintReadOnly) float NextTickTime = 0.0f;
};

/**
 * UStatusEffectComponent
 *
 * Attach to any character to support damage-over-time effects.
 * Burn: fire damage every 0.5s for 5s.
 * Poison: lower damage but longer duration, slows too.
 * Bleed: stacks with each hit, scales with enemy count.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusEffectComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Anansi|StatusEffect")
	void ApplyEffect(EStatusEffectType Type, float Duration = 5.0f, float DamagePerTick = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Anansi|StatusEffect")
	void ClearAllEffects();

	UFUNCTION(BlueprintPure, Category = "Anansi|StatusEffect")
	bool HasEffect(EStatusEffectType Type) const;

private:
	UPROPERTY()
	TArray<FStatusEffect> ActiveEffects;

	void ProcessEffect(FStatusEffect& Effect, float DeltaTime);
};
