// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "AnansiCharacter.generated.h"

class UCombatComponent;
class UWeaponTraceComponent;
class UMeleeDamageDealer;
class UAnansiInteractionComponent;
class UStaticMeshComponent;
class UPostProcessComponent;
class UAbilitySystemComponent;
class UGameplayAbility;
class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

// ---------------------------------------------------------------------------
// Character state machine
// ---------------------------------------------------------------------------

/**
 * High-level character state. Drives animation-layer selection, input routing,
 * and determines which abilities/moves are available at any moment.
 */
UENUM(BlueprintType)
enum class EAnansiCharacterState : uint8
{
	Idle			UMETA(DisplayName = "Idle"),
	Moving			UMETA(DisplayName = "Moving"),
	Jumping			UMETA(DisplayName = "Jumping"),
	WallRunning		UMETA(DisplayName = "Wall Running"),
	WebSwinging		UMETA(DisplayName = "Web Swinging"),
	Attacking		UMETA(DisplayName = "Attacking"),
	Dodging			UMETA(DisplayName = "Dodging"),
	Crouching		UMETA(DisplayName = "Crouching"),
	Stealth			UMETA(DisplayName = "Stealth"),
	Parrying		UMETA(DisplayName = "Parrying"),
	UsingAbility	UMETA(DisplayName = "Using Ability"),
	Stunned			UMETA(DisplayName = "Stunned"),
	Dead			UMETA(DisplayName = "Dead")
};

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebEnergyChanged, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterStateChanged, EAnansiCharacterState, OldState, EAnansiCharacterState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpiderSenseTriggered, AActor*, ThreatActor);

/**
 * AAnansiCharacter
 *
 * The main player-controlled character for Anansi: Web of Shadows.
 *
 * Design pillars reflected here:
 *  - Multi-modal traversal (ground, wall-run, web-swing) via state machine.
 *  - Deep combo-driven combat delegated to UCombatComponent.
 *  - Stealth layer with crouch / visibility tracking.
 *  - Gameplay Ability System (GAS) integration for special powers.
 *  - Spider Sense proximity detection via overlap sphere.
 *  - Resource management: Health, Web Energy (mana analogue), Stamina.
 *
 * Inputs are bound through UE5 Enhanced Input System — see SetupPlayerInputComponent.
 */
UCLASS(config = Game)
class ANANSIGAME_API AAnansiCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAnansiCharacter();

	// -----------------------------------------------------------------------
	// AActor / ACharacter overrides
	// -----------------------------------------------------------------------

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// -----------------------------------------------------------------------
	// Components (public for blueprint/C++ access)
	// -----------------------------------------------------------------------

	/** Third-person camera boom. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** Central combat logic — combos, dodge, parry, lock-on. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Combat")
	TObjectPtr<UCombatComponent> CombatComponent;

	/** Weapon hit-detection traces (driven by anim notifies). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Combat")
	TObjectPtr<UWeaponTraceComponent> WeaponTrace;

	/** Proximity-based melee damage (works without weapon mesh). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Combat")
	TObjectPtr<UMeleeDamageDealer> MeleeDamage;

	/** Gameplay Ability System component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;

	/** Interaction detection and prompt management. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Interaction")
	TObjectPtr<UAnansiInteractionComponent> InteractionComponent;

	/** Visible capsule mesh so the player can see themselves without a skeletal mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Visual")
	TObjectPtr<UStaticMeshComponent> PlayerVisualMesh;

	/** Post-processing for combat effects (hit flash, low-health vignette). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Visual")
	TObjectPtr<UPostProcessComponent> PostProcessComp;

	/** Spider Sense detection sphere — overlaps trigger threat warnings. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|SpiderSense")
	TObjectPtr<USphereComponent> SpiderSenseSphere;

	// -----------------------------------------------------------------------
	// State machine
	// -----------------------------------------------------------------------

	/** Get the current character state. */
	UFUNCTION(BlueprintPure, Category = "Anansi|State")
	EAnansiCharacterState GetCharacterState() const { return CurrentState; }

	/** Request a state transition. Will validate the transition before applying. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|State")
	void SetCharacterState(EAnansiCharacterState NewState);

	UPROPERTY(BlueprintAssignable, Category = "Anansi|State")
	FOnCharacterStateChanged OnCharacterStateChanged;

	// -----------------------------------------------------------------------
	// Health
	// -----------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void ApplyDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void ApplyHealing(float Amount);

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	/** Reset the player to full health and return to spawn point. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void Respawn();

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Stats")
	FOnHealthChanged OnHealthChanged;

	// -----------------------------------------------------------------------
	// Web Energy (mana analogue for web-based abilities)
	// -----------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	float GetCurrentWebEnergy() const { return CurrentWebEnergy; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	float GetMaxWebEnergy() const { return MaxWebEnergy; }

	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	bool ConsumeWebEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	void RestoreWebEnergy(float Amount);

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Stats")
	FOnWebEnergyChanged OnWebEnergyChanged;

	// -----------------------------------------------------------------------
	// Stamina
	// -----------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Stats")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintCallable, Category = "Anansi|Stats")
	bool ConsumeStamina(float Amount);

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Stats")
	FOnStaminaChanged OnStaminaChanged;

	// -----------------------------------------------------------------------
	// Combo counter (exposed for HUD)
	// -----------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Combat")
	int32 GetComboCount() const;

	// -----------------------------------------------------------------------
	// Spider Sense
	// -----------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|SpiderSense")
	FOnSpiderSenseTriggered OnSpiderSenseTriggered;

	/** Radius of the Spider Sense detection sphere. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|SpiderSense", meta = (ClampMin = "100.0"))
	float SpiderSenseRadius = 800.0f;

	// -----------------------------------------------------------------------
	// Traversal configuration
	// -----------------------------------------------------------------------

	/** Wall-run: minimum speed required to initiate a wall run. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Traversal", meta = (ClampMin = "0.0"))
	float WallRunMinSpeed = 400.0f;

	/** Wall-run: maximum duration before the character detaches. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Traversal", meta = (ClampMin = "0.0"))
	float WallRunMaxDuration = 2.0f;

	/** Web-swing: energy cost per second of swinging. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Traversal", meta = (ClampMin = "0.0"))
	float WebSwingEnergyCostPerSecond = 10.0f;

	// -----------------------------------------------------------------------
	// GAS — initial abilities
	// -----------------------------------------------------------------------

	/** Abilities granted on spawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> InitialAbilities;

protected:
	// -----------------------------------------------------------------------
	// Enhanced Input actions (bound in blueprint or via DefaultInput.ini)
	// -----------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_LightAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_HeavyAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_Dodge;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_Parry;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_Crouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_LockOn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_WebSwing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_Interact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_SpiderSense;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_SilkenPath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_TrickMirror;

	// -----------------------------------------------------------------------
	// Input callbacks
	// -----------------------------------------------------------------------

	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_Jump(const FInputActionValue& Value);
	void Input_StopJumping(const FInputActionValue& Value);
	void Input_LightAttack(const FInputActionValue& Value);
	void Input_HeavyAttack(const FInputActionValue& Value);
	void Input_Dodge(const FInputActionValue& Value);
	void Input_Parry(const FInputActionValue& Value);
	void Input_CrouchToggle(const FInputActionValue& Value);
	void Input_SprintStart(const FInputActionValue& Value);
	void Input_SprintStop(const FInputActionValue& Value);
	void Input_Interact(const FInputActionValue& Value);
	void Input_LockOn(const FInputActionValue& Value);
	void Input_WebSwing(const FInputActionValue& Value);
	void Input_WebSwingRelease(const FInputActionValue& Value);
	void Input_SpiderSense(const FInputActionValue& Value);
	void Input_SilkenPath(const FInputActionValue& Value);
	void Input_TrickMirror(const FInputActionValue& Value);
	void Input_GroundSlam(const FInputActionValue& Value);

	// -----------------------------------------------------------------------
	// State machine helpers
	// -----------------------------------------------------------------------

	/** Called every tick to evaluate state transitions driven by physics/movement. */
	void UpdateStateFromMovement(float DeltaTime);

	/** Check if we should initiate a wall-run based on a wall trace. */
	bool TryStartWallRun();

	/** Tick logic while wall-running. */
	void TickWallRun(float DeltaTime);

	/** Tick logic while web-swinging. */
	void TickWebSwing(float DeltaTime);

	// -----------------------------------------------------------------------
	// Spider Sense overlap callbacks
	// -----------------------------------------------------------------------

	UFUNCTION()
	void OnSpiderSenseBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// -----------------------------------------------------------------------
	// GAS helpers
	// -----------------------------------------------------------------------

	void GrantInitialAbilities();

private:
	// -- State machine ------------------------------------------------------
	EAnansiCharacterState CurrentState = EAnansiCharacterState::Idle;

	// -- Resources ----------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Stats", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Anansi|Stats")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Stats", meta = (ClampMin = "1.0"))
	float MaxWebEnergy = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Anansi|Stats")
	float CurrentWebEnergy = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Stats", meta = (ClampMin = "1.0"))
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Anansi|Stats")
	float CurrentStamina = 100.0f;

	/** Stamina regeneration rate per second. */
	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Stats", meta = (ClampMin = "0.0"))
	float StaminaRegenRate = 15.0f;

	/** Delay after stamina use before regeneration begins. */
	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Stats", meta = (ClampMin = "0.0"))
	float StaminaRegenDelay = 1.0f;

	float StaminaRegenCooldown = 0.0f;

	/** Web energy passive regeneration per second. */
	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Stats", meta = (ClampMin = "0.0"))
	float WebEnergyRegenRate = 3.0f;

	/** Auto-load Input Actions and Mapping Context from /Game/Input/ if not set. */
	void AutoLoadInputAssets();

	// -- Wall-run state -----------------------------------------------------
	float WallRunTimer = 0.0f;
	FVector WallRunNormal = FVector::ZeroVector;

	// -- Web-swing state ----------------------------------------------------
	bool bIsWebSwinging = false;
	FVector WebAnchorPoint = FVector::ZeroVector;

	// -- Double jump --------------------------------------------------------
	int32 JumpCount = 0;
	int32 MaxJumpCount = 2;

	/** Height threshold for fall damage (units). */
	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Movement", meta = (ClampMin = "0.0"))
	float FallDamageThreshold = 800.0f;

	/** Damage per unit fallen beyond threshold. */
	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Movement", meta = (ClampMin = "0.0"))
	float FallDamagePerUnit = 0.05f;

	float FallStartZ = 0.0f;
	bool bIsFallingFromHeight = false;

	// -- Sprint state -------------------------------------------------------
	bool bIsSprinting = false;

	/** Walk speed restored when sprint ends. */
	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Movement", meta = (ClampMin = "0.0"))
	float DefaultWalkSpeed = 600.0f;

	/** Speed while sprinting. */
	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Movement", meta = (ClampMin = "0.0"))
	float SprintSpeed = 900.0f;

	/** Stamina cost per second while sprinting. */
	UPROPERTY(EditDefaultsOnly, Category = "Anansi|Movement", meta = (ClampMin = "0.0"))
	float SprintStaminaCostPerSecond = 12.0f;

	// -- Wall slide ---------------------------------------------------------
	bool bIsWallSliding = false;
	FVector WallSlideNormal = FVector::ZeroVector;

	void CheckWallSlide();

	// -- Footstep sounds ----------------------------------------------------
	float FootstepTimer = 0.0f;
	float FootstepInterval = 0.35f;

	void PlayFootstepSound();

	// -- Trick Mirror decoy -------------------------------------------------
	TWeakObjectPtr<AActor> ActiveDecoy;

	// -- Stealth / crouch ---------------------------------------------------
	bool bIsCrouching = false;
};
