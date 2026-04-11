// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/DamageTypes.h"
#include "CombatComponent.generated.h"

class UComboChainData;
class UWeaponTraceComponent;
class UAnimMontage;

// ---------------------------------------------------------------------------
// Style rank — awarded for varied, uninterrupted combat
// ---------------------------------------------------------------------------

UENUM(BlueprintType)
enum class EStyleRank : uint8
{
	D		UMETA(DisplayName = "D - Dull"),
	C		UMETA(DisplayName = "C - Cool"),
	B		UMETA(DisplayName = "B - Badass"),
	A		UMETA(DisplayName = "A - Awesome"),
	S		UMETA(DisplayName = "S - Stylish"),
	SS		UMETA(DisplayName = "SS - Sensational"),
	SSS		UMETA(DisplayName = "SSS - Smokin' Sexy Style")
};

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboCountChanged, int32, OldCount, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStyleRankChanged, EStyleRank, OldRank, EStyleRank, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageReceived, const FAnansiDamageInfo&, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParrySuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDodgeSuccess);

/**
 * UCombatComponent
 *
 * Central hub for all combat mechanics on the owning character.
 *
 * Responsibilities:
 *  - Light/heavy attack combo chains driven by UComboChainData assets.
 *  - Dodge/roll with configurable i-frame window.
 *  - Parry/counter with strict timing.
 *  - Lock-on target acquisition and switching.
 *  - Combo counter with decay, feeding into a style-rank system (D to SSS).
 *  - Hit-stop (localised time dilation) for impactful hits.
 *  - Camera shake on hits.
 *
 * This component does NOT own health/energy — those live on the character.
 * It communicates damage through delegates and direct function calls.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class ANANSIGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// -----------------------------------------------------------------------
	// Attack interface
	// -----------------------------------------------------------------------

	/** Request a light attack. Advances the light combo chain if timing is valid. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Combat")
	void RequestLightAttack();

	/** Request a heavy (charged) attack. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Combat")
	void RequestHeavyAttack();

	/** Notify that the current attack montage has ended (bound in BeginPlay). */
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** Notify that a weapon trace scored a hit — called by the weapon trace component. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Combat")
	void OnWeaponHitDetected(const FVector& HitLocation, const FVector& HitNormal, AActor* HitTarget);

	// -----------------------------------------------------------------------
	// Defence interface
	// -----------------------------------------------------------------------

	/** Begin dodge/roll. Grants i-frames for DodgeIFrameDuration. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Combat")
	void StartDodge();

	/** Begin parry window. Must be timed to an incoming attack. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Combat")
	void StartParry();

	/**
	 * Process incoming damage. Checks for i-frames and parry state.
	 * Returns the final damage dealt after mitigation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Combat")
	float ReceiveDamage(const FAnansiDamageInfo& DamageInfo);

	/** Is the character currently invulnerable (dodge i-frames)? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Combat")
	bool IsInvulnerable() const { return bIsInvulnerable; }

	/** Is the character currently in the parry window? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Combat")
	bool IsParrying() const { return bIsParrying; }

	/** Is the character mid-attack (any combo)? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Combat")
	bool IsAttacking() const { return bIsAttacking; }

	// -----------------------------------------------------------------------
	// Lock-on
	// -----------------------------------------------------------------------

	/** Toggle lock-on to the nearest valid target. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Combat")
	void ToggleLockOn();

	/** Switch lock-on to the next target in the given direction (-1 = left, +1 = right). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Combat")
	void SwitchLockOnTarget(float Direction);

	/** Get the current lock-on target (may be nullptr). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Combat")
	AActor* GetLockOnTarget() const { return LockOnTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "Anansi|Combat")
	bool IsLockedOn() const { return LockOnTarget.IsValid(); }

	// -----------------------------------------------------------------------
	// Combo / Style
	// -----------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Combat")
	int32 GetComboCount() const { return ComboCount; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Combat")
	EStyleRank GetStyleRank() const { return CurrentStyleRank; }

	/** Reset the combo counter (called on getting hit or after decay). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Combat")
	void ResetCombo();

	/** Get the variety multiplier for the current combo (1.0-1.5). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Combat")
	float GetComboVarietyMultiplier() const;

	// -----------------------------------------------------------------------
	// Configuration — combo data
	// -----------------------------------------------------------------------

	/** Data asset defining the light attack combo chain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Config")
	TObjectPtr<UComboChainData> LightComboData;

	/** Data asset defining the heavy attack chain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Config")
	TObjectPtr<UComboChainData> HeavyComboData;

	// -----------------------------------------------------------------------
	// Configuration — timing
	// -----------------------------------------------------------------------

	/** How long dodge i-frames last (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Config", meta = (ClampMin = "0.0"))
	float DodgeIFrameDuration = 0.3f;

	/** How long the parry window stays open (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Config", meta = (ClampMin = "0.0"))
	float ParryWindowDuration = 0.2f;

	/** Seconds of no hits before the combo counter starts decaying. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Config", meta = (ClampMin = "0.0"))
	float ComboDecayDelay = 3.0f;

	/** Rate at which the combo counter decreases per second after decay starts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Config", meta = (ClampMin = "0.0"))
	float ComboDecayRate = 5.0f;

	// -----------------------------------------------------------------------
	// Configuration — lock-on
	// -----------------------------------------------------------------------

	/** Maximum distance for lock-on target acquisition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|LockOn", meta = (ClampMin = "0.0"))
	float LockOnMaxDistance = 2000.0f;

	/** Half-angle (degrees) of the lock-on cone in front of the character. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|LockOn", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float LockOnConeHalfAngle = 45.0f;

	// -----------------------------------------------------------------------
	// Configuration — hit-stop & camera shake
	// -----------------------------------------------------------------------

	/** Duration of hit-stop time dilation on impactful hits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Feel", meta = (ClampMin = "0.0"))
	float HitStopDuration = 0.05f;

	/** Time dilation scale during hit-stop (0 = full freeze). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Feel", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HitStopTimeDilation = 0.01f;

	/** Camera shake class played on dealing damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Feel")
	TSubclassOf<UCameraShakeBase> HitCameraShake;

	/** Camera shake class played on receiving damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Feel")
	TSubclassOf<UCameraShakeBase> DamageCameraShake;

	// -----------------------------------------------------------------------
	// Dodge montage
	// -----------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Animations")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Animations")
	TObjectPtr<UAnimMontage> ParryMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Combat|Animations")
	TObjectPtr<UAnimMontage> ParryCounterMontage;

	// -----------------------------------------------------------------------
	// Delegates
	// -----------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Combat")
	FOnComboCountChanged OnComboCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Combat")
	FOnStyleRankChanged OnStyleRankChanged;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Combat")
	FOnDamageReceived OnDamageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Combat")
	FOnParrySuccess OnParrySuccess;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Combat")
	FOnDodgeSuccess OnDodgeSuccess;

private:
	// -- Attack state -------------------------------------------------------
	bool bIsAttacking = false;
	int32 LightComboIndex = 0;
	int32 HeavyComboIndex = 0;
	bool bCanChainNextHit = false;
	bool bNextAttackQueued = false;

	/** Play the montage for the given combo index. Returns true on success. */
	bool PlayComboHit(UComboChainData* ComboData, int32 Index);

	// -- Defence state ------------------------------------------------------
	bool bIsInvulnerable = false;
	bool bIsParrying = false;
	FTimerHandle IFrameTimerHandle;
	FTimerHandle ParryTimerHandle;

	void EndIFrames();
	void EndParry();

	// -- Lock-on ------------------------------------------------------------
	TWeakObjectPtr<AActor> LockOnTarget;
	TArray<TWeakObjectPtr<AActor>> LockOnCandidates;
	void RefreshLockOnCandidates();

	// -- Combo / Style ------------------------------------------------------
	int32 ComboCount = 0;
	float TimeSinceLastHit = 0.0f;
	EStyleRank CurrentStyleRank = EStyleRank::D;

	/** Tracks attack type variety in the current combo (L vs H). */
	int32 LightHitsInCombo = 0;
	int32 HeavyHitsInCombo = 0;

	/** Variety bonus multiplier (1.0 = no variety, up to 1.5 with good mix). */
	float GetVarietyMultiplier() const;

	void IncrementCombo();
	EStyleRank CalculateStyleRank(int32 Combo) const;

	// -- Hit-stop -----------------------------------------------------------
	FTimerHandle HitStopTimerHandle;
	void ApplyHitStop(float Duration, float Dilation);
	void EndHitStop();
};
