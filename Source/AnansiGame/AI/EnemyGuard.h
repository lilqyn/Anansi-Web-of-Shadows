// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyBase.h"
#include "EnemyGuard.generated.h"

class USimpleEnemyAI;

/**
 * Guard variant — melee or shield.
 */
UENUM(BlueprintType)
enum class EGuardVariant : uint8
{
	Melee	UMETA(DisplayName = "Melee"),
	Shield	UMETA(DisplayName = "Shield")
};

/**
 * AEnemyGuard
 *
 * Palace Guard enemy for the vertical slice. Two variants:
 *
 * Melee Guard:
 *  - Standard attack patterns (swing, thrust)
 *  - Moderate health, normal stagger threshold
 *  - Patrols waypoints, investigates sounds
 *
 * Shield Guard:
 *  - Blocks frontal attacks with shield (reduced damage from front)
 *  - Must be flanked, parried, or staggered to open up
 *  - Slower attacks but hits harder
 *  - Higher health pool
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AEnemyGuard : public AEnemyBase
{
	GENERATED_BODY()

public:
	AEnemyGuard();

	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Anansi|Guard")
	EGuardVariant GetGuardVariant() const { return GuardVariant; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Guard")
	bool IsBlocking() const { return bIsBlocking; }

	/** Force the guard to raise or lower their shield. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Guard")
	void SetBlocking(bool bBlock);

	/** Check if an incoming attack hits the shield (frontal cone). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Guard")
	bool IsAttackBlockedByShield(const FVector& AttackDirection) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Guard")
	EGuardVariant GuardVariant = EGuardVariant::Melee;

	/** Damage reduction multiplier when blocking (0 = full block, 1 = no reduction). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Guard|Shield",
		meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "GuardVariant == EGuardVariant::Shield"))
	float ShieldDamageReduction = 0.15f;

	/** Half-angle of the shield's frontal protection cone (degrees). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Guard|Shield",
		meta = (ClampMin = "0.0", ClampMax = "180.0", EditCondition = "GuardVariant == EGuardVariant::Shield"))
	float ShieldConeHalfAngle = 60.0f;

	/** Stamina cost per blocked hit. Shield drops when depleted. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Guard|Shield",
		meta = (ClampMin = "0.0", EditCondition = "GuardVariant == EGuardVariant::Shield"))
	float BlockStaminaCost = 15.0f;

	/** Max shield stamina — regenerates when not blocking. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Guard|Shield",
		meta = (ClampMin = "1.0", EditCondition = "GuardVariant == EGuardVariant::Shield"))
	float MaxShieldStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Guard")
	TObjectPtr<USimpleEnemyAI> EnemyAI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Guard|Animation")
	TObjectPtr<UAnimMontage> BlockImpactMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Guard|Animation")
	TObjectPtr<UAnimMontage> ShieldBreakMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Guard|Animation")
	TObjectPtr<UAnimMontage> AttackMontage_Swing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Guard|Animation")
	TObjectPtr<UAnimMontage> AttackMontage_Thrust;

private:
	bool bIsBlocking = false;
	float CurrentShieldStamina = 100.0f;

	void InitMeleeVariant();
	void InitShieldVariant();
};
