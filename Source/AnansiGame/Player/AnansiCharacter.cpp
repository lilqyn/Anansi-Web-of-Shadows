// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Player/AnansiCharacter.h"
#include "AnansiGame.h"
#include "Combat/CombatComponent.h"
#include "Combat/ComboData.h"
#include "Combat/WeaponTrace.h"
#include "Combat/MeleeDamageDealer.h"
#include "Combat/AnansiCameraShake.h"
#include "World/InteractionComponent.h"
#include "UI/AnansiDevHUD.h"
#include "AI/SimpleEnemyAI.h"
#include "Core/CombatStats.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "World/Checkpoint.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

AAnansiCharacter::AAnansiCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// -- Camera boom --------------------------------------------------------
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// -- Movement defaults --------------------------------------------------
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 250.0f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// -- Visual mesh (capsule so you can see yourself) ---------------------
	PlayerVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerVisualMesh"));
	PlayerVisualMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		PlayerVisualMesh->SetStaticMesh(CylinderMesh.Object);
		PlayerVisualMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.0f));
		PlayerVisualMesh->SetRelativeLocation(FVector(0, 0, -88));
		PlayerVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PlayerVisualMesh->CastShadow = true;
	}

	// -- Post-processing for combat effects --------------------------------
	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcessComp->SetupAttachment(RootComponent);
	PostProcessComp->bUnbound = true;
	PostProcessComp->Priority = 1.0f;

	// -- Combat components --------------------------------------------------
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	WeaponTrace = CreateDefaultSubobject<UWeaponTraceComponent>(TEXT("WeaponTrace"));
	MeleeDamage = CreateDefaultSubobject<UMeleeDamageDealer>(TEXT("MeleeDamage"));

	// -- Interaction --------------------------------------------------------
	InteractionComponent = CreateDefaultSubobject<UAnansiInteractionComponent>(TEXT("InteractionComponent"));

	// -- Ability system -----------------------------------------------------
	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));

	// -- Spider Sense detection sphere --------------------------------------
	SpiderSenseSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SpiderSenseSphere"));
	SpiderSenseSphere->SetupAttachment(RootComponent);
	SpiderSenseSphere->SetSphereRadius(SpiderSenseRadius);
	SpiderSenseSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SpiderSenseSphere->SetGenerateOverlapEvents(true);
	SpiderSenseSphere->SetHiddenInGame(true);

	// -- Resource defaults --------------------------------------------------
	CurrentHealth = MaxHealth;
	CurrentWebEnergy = MaxWebEnergy;
	CurrentStamina = MaxStamina;

	// -- Create input actions and mapping context in constructor so they
	//    exist before SetupPlayerInputComponent runs.
	AutoLoadInputAssets();
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void AAnansiCharacter::BeginPlay()
{
	Super::BeginPlay();

	// -- Auto-load combo data if not set ------------------------------------
	if (CombatComponent)
	{
		if (!CombatComponent->LightComboData)
		{
			CombatComponent->LightComboData = LoadObject<UComboChainData>(
				nullptr, TEXT("/Game/Combat/ComboData/CDC_LightCombo.CDC_LightCombo"));
		}
		if (!CombatComponent->HeavyComboData)
		{
			CombatComponent->HeavyComboData = LoadObject<UComboChainData>(
				nullptr, TEXT("/Game/Combat/ComboData/CDC_HeavyCombo.CDC_HeavyCombo"));
		}
	}

	// Register spider-sense overlap.
	SpiderSenseSphere->OnComponentBeginOverlap.AddDynamic(this, &AAnansiCharacter::OnSpiderSenseBeginOverlap);

	// Apply mapping context.
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	GrantInitialAbilities();
}

void AAnansiCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// GAS needs the owner/avatar set after possession.
	if (AbilitySystemComp)
	{
		AbilitySystemComp->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* AAnansiCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void AAnansiCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// -- Stamina regeneration -----------------------------------------------
	if (StaminaRegenCooldown > 0.0f)
	{
		StaminaRegenCooldown -= DeltaTime;
	}
	else if (CurrentStamina < MaxStamina)
	{
		const float OldStamina = CurrentStamina;
		CurrentStamina = FMath::Min(CurrentStamina + StaminaRegenRate * DeltaTime, MaxStamina);
		if (CurrentStamina != OldStamina)
		{
			OnStaminaChanged.Broadcast(OldStamina, CurrentStamina);
		}
	}

	// -- Web energy passive regen -------------------------------------------
	if (CurrentWebEnergy < MaxWebEnergy)
	{
		const float OldEnergy = CurrentWebEnergy;
		CurrentWebEnergy = FMath::Min(CurrentWebEnergy + WebEnergyRegenRate * DeltaTime, MaxWebEnergy);
		if (CurrentWebEnergy != OldEnergy)
		{
			OnWebEnergyChanged.Broadcast(OldEnergy, CurrentWebEnergy);
		}
	}

	// -- Sprint stamina drain -----------------------------------------------
	if (bIsSprinting && CurrentStamina > 0.0f)
	{
		if (!ConsumeStamina(SprintStaminaCostPerSecond * DeltaTime))
		{
			bIsSprinting = false;
			GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
		}
	}

	// -- Footstep sounds ----------------------------------------------------
	if (!GetCharacterMovement()->IsFalling() && GetVelocity().Size2D() > 50.0f)
	{
		const float SpeedRatio = GetVelocity().Size2D() / SprintSpeed;
		const float Interval = FMath::Lerp(0.45f, 0.25f, SpeedRatio);
		FootstepTimer += DeltaTime;
		if (FootstepTimer >= Interval)
		{
			FootstepTimer = 0.0f;
			PlayFootstepSound();
		}
	}
	else
	{
		FootstepTimer = 0.0f;
	}

	// -- Sprint trail lines -------------------------------------------------
#if ENABLE_DRAW_DEBUG
	if (bIsSprinting && GetVelocity().Size() > 500.0f)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(),
			GetActorLocation() - GetVelocity().GetSafeNormal() * 100.0f,
			FColor(100, 200, 255, 80), false, 0.15f, 0, 1.5f);
	}
#endif

	// -- Sprint FOV change --------------------------------------------------
	if (FollowCamera)
	{
		const float TargetFOV = bIsSprinting ? 100.0f : 90.0f;
		const float CurrentFOV = FollowCamera->FieldOfView;
		FollowCamera->SetFieldOfView(FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 6.0f));
	}

	// -- Wall slide check ---------------------------------------------------
	CheckWallSlide();

	// -- Fall damage tracking -----------------------------------------------
	if (GetCharacterMovement()->IsFalling())
	{
		if (!bIsFallingFromHeight)
		{
			bIsFallingFromHeight = true;
			FallStartZ = GetActorLocation().Z;
		}
	}
	else if (bIsFallingFromHeight)
	{
		bIsFallingFromHeight = false;
		JumpCount = 0; // Reset double jump on landing

		const float FallDistance = FallStartZ - GetActorLocation().Z;
		if (FallDistance > FallDamageThreshold)
		{
			const float Damage = (FallDistance - FallDamageThreshold) * FallDamagePerUnit;
			ApplyDamage(Damage);
			UE_LOG(LogAnansi, Log, TEXT("Fall damage: %.0f units fallen, %.1f damage"), FallDistance, Damage);

			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				PC->ClientStartCameraShake(UAnansiDamageShake::StaticClass());
			}
		}
	}

	// -- Low-health vignette effect ----------------------------------------
	if (PostProcessComp && MaxHealth > 0.0f)
	{
		const float HealthPct = CurrentHealth / MaxHealth;
		if (HealthPct < 0.3f)
		{
			const float Intensity = FMath::Lerp(1.5f, 0.0f, HealthPct / 0.3f);
			PostProcessComp->Settings.bOverride_VignetteIntensity = true;
			PostProcessComp->Settings.VignetteIntensity = Intensity;

			// Red tint when very low
			PostProcessComp->Settings.bOverride_SceneColorTint = true;
			const float RedTint = FMath::Lerp(1.2f, 1.0f, HealthPct / 0.3f);
			PostProcessComp->Settings.SceneColorTint = FLinearColor(RedTint, 1.0f, 1.0f);
		}
		else
		{
			PostProcessComp->Settings.bOverride_VignetteIntensity = false;
			PostProcessComp->Settings.bOverride_SceneColorTint = false;
		}
	}

	// -- Ability active glow on player mesh ---------------------------------
	if (PlayerVisualMesh)
	{
		const bool bAbilityActive = (CurrentState == EAnansiCharacterState::UsingAbility) || bIsWebSwinging;
		PlayerVisualMesh->SetRenderCustomDepth(bAbilityActive);
		if (bAbilityActive)
		{
			PlayerVisualMesh->SetCustomDepthStencilValue(2); // Cyan glow
		}
	}

	// -- State machine updates driven by physics ----------------------------
	UpdateStateFromMovement(DeltaTime);

	// -- Traversal-specific ticks -------------------------------------------
	if (CurrentState == EAnansiCharacterState::WallRunning)
	{
		TickWallRun(DeltaTime);
	}
	else if (CurrentState == EAnansiCharacterState::WebSwinging)
	{
		TickWebSwing(DeltaTime);
	}
}

// ---------------------------------------------------------------------------
// Enhanced Input binding
// ---------------------------------------------------------------------------

void AAnansiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		UE_LOG(LogAnansi, Error, TEXT("AnansiCharacter requires EnhancedInputComponent."));
		return;
	}

	// Movement
	if (IA_Move)    EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AAnansiCharacter::Input_Move);
	if (IA_Look)    EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AAnansiCharacter::Input_Look);
	if (IA_Jump)
	{
		EIC->BindAction(IA_Jump, ETriggerEvent::Started,   this, &AAnansiCharacter::Input_Jump);
		EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AAnansiCharacter::Input_StopJumping);
	}

	// Combat
	if (IA_LightAttack) EIC->BindAction(IA_LightAttack, ETriggerEvent::Started, this, &AAnansiCharacter::Input_LightAttack);
	if (IA_HeavyAttack) EIC->BindAction(IA_HeavyAttack, ETriggerEvent::Started, this, &AAnansiCharacter::Input_HeavyAttack);
	if (IA_Dodge)       EIC->BindAction(IA_Dodge,       ETriggerEvent::Started, this, &AAnansiCharacter::Input_Dodge);
	if (IA_Parry)       EIC->BindAction(IA_Parry,       ETriggerEvent::Started, this, &AAnansiCharacter::Input_Parry);
	if (IA_LockOn)      EIC->BindAction(IA_LockOn,      ETriggerEvent::Started, this, &AAnansiCharacter::Input_LockOn);

	// Traversal
	if (IA_Crouch)    EIC->BindAction(IA_Crouch,    ETriggerEvent::Started, this, &AAnansiCharacter::Input_CrouchToggle);
	if (IA_WebSwing)
	{
		EIC->BindAction(IA_WebSwing, ETriggerEvent::Started,   this, &AAnansiCharacter::Input_WebSwing);
		EIC->BindAction(IA_WebSwing, ETriggerEvent::Completed, this, &AAnansiCharacter::Input_WebSwingRelease);
	}
	if (IA_Sprint)
	{
		EIC->BindAction(IA_Sprint, ETriggerEvent::Started,   this, &AAnansiCharacter::Input_SprintStart);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AAnansiCharacter::Input_SprintStop);
	}

	// Interaction
	if (IA_Interact)    EIC->BindAction(IA_Interact,    ETriggerEvent::Started, this, &AAnansiCharacter::Input_Interact);

	// Abilities
	if (IA_SpiderSense) EIC->BindAction(IA_SpiderSense, ETriggerEvent::Started, this, &AAnansiCharacter::Input_SpiderSense);
	if (IA_SilkenPath)  EIC->BindAction(IA_SilkenPath,  ETriggerEvent::Started, this, &AAnansiCharacter::Input_SilkenPath);
	if (IA_TrickMirror) EIC->BindAction(IA_TrickMirror, ETriggerEvent::Started, this, &AAnansiCharacter::Input_TrickMirror);
}

// ---------------------------------------------------------------------------
// Input handlers
// ---------------------------------------------------------------------------

void AAnansiCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D MoveInput = Value.Get<FVector2D>();

	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	// Calculate movement direction relative to the camera.
	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, MoveInput.Y);
	AddMovementInput(RightDir,   MoveInput.X);
}

void AAnansiCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();
	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void AAnansiCharacter::Input_Jump(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	// If near a wall and moving fast enough, try wall-run instead of jump.
	if (GetCharacterMovement()->IsFalling() && TryStartWallRun())
	{
		return;
	}

	// Wall kick-off — jump away from wall while wall-sliding
	if (bIsWallSliding)
	{
		bIsWallSliding = false;
		GetCharacterMovement()->GravityScale = 1.0f;
		LaunchCharacter(WallSlideNormal * 500.0f + FVector(0, 0, 600), true, true);
		JumpCount = 1; // Allow one more double jump after wall kick
		SetCharacterState(EAnansiCharacterState::Jumping);
		UE_LOG(LogAnansi, Verbose, TEXT("Wall kick-off!"));
		return;
	}

	// Ground jump
	if (!GetCharacterMovement()->IsFalling())
	{
		JumpCount = 0;
		FallStartZ = GetActorLocation().Z;
		Jump();
		JumpCount = 1;
		return;
	}

	// Double jump (airborne)
	if (JumpCount < MaxJumpCount)
	{
		// Reset velocity and apply jump
		FVector Vel = GetVelocity();
		Vel.Z = GetCharacterMovement()->JumpZVelocity * 0.85f;
		GetCharacterMovement()->Velocity = Vel;
		JumpCount++;
	}
}

void AAnansiCharacter::Input_StopJumping(const FInputActionValue& Value)
{
	StopJumping();
}

void AAnansiCharacter::Input_LightAttack(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	if (CombatComponent)
	{
		CombatComponent->RequestLightAttack();
		SetCharacterState(EAnansiCharacterState::Attacking);
	}

	// -- Stealth takedown check (crouching + behind enemy) -----------------
	if (bIsCrouching && MeleeDamage)
	{
		// Check if there's an enemy nearby that we're behind
		TArray<AActor*> Enemies;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

		for (AActor* Enemy : Enemies)
		{
			if (!Enemy || Enemy == this) continue;
			const float Dist = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
			if (Dist > 250.0f) continue;

			// Check if we're behind them
			const FVector ToPlayer = (GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
			const float Dot = FVector::DotProduct(Enemy->GetActorForwardVector(), ToPlayer);
			if (Dot > 0.3f) // Behind or to the side-rear
			{
				// Instant kill takedown
				FDamageEvent DamageEvent;
				Enemy->TakeDamage(99999.0f, DamageEvent, GetController(), this);
				UE_LOG(LogAnansi, Log, TEXT("Stealth takedown on %s!"), *Enemy->GetName());

				// Brief slow-motion for cinematic feel
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
				FTimerHandle SlowMoTimer;
				GetWorldTimerManager().SetTimer(SlowMoTimer, [this]()
				{
					UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
				}, 0.06f, false); // Real time = 0.06s, perceived = 0.3s

				SetCharacterState(EAnansiCharacterState::Attacking);
				FTimerHandle AttackResetTimer;
				GetWorldTimerManager().SetTimer(AttackResetTimer, [this]()
				{
					if (CurrentState == EAnansiCharacterState::Attacking)
						SetCharacterState(EAnansiCharacterState::Crouching);
				}, 0.5f, false);
				return; // Don't do normal attack
			}
		}
	}

	// Fire proximity-based melee damage — scales with combo count.
	if (MeleeDamage)
	{
		const int32 ComboHits = CombatComponent ? CombatComponent->GetComboCount() : 0;
		float Damage = 15.0f;

		// Combo finisher bonus every 3 hits
		if (ComboHits > 0 && ComboHits % 3 == 0)
		{
			Damage = 30.0f; // Finisher hit
			MeleeDamage->KnockbackForce = 700.0f;

			// Show toast
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
				{
					HUD->ShowToast(TEXT("COMBO FINISHER!"), FColor(255, 150, 0));
				}
			}
		}

		// Variety multiplier
		const float VarietyMult = CombatComponent ? CombatComponent->GetComboVarietyMultiplier() : 1.0f;
		Damage *= VarietyMult;

		const int32 Hits = MeleeDamage->FireAttack(Damage);
		MeleeDamage->KnockbackForce = 400.0f; // Reset

		if (Hits > 0)
		{
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				PC->ClientStartCameraShake(UAnansiHitShake::StaticClass());
			}
		}
	}

	// Auto-return to idle after a short delay (no montage to end the state).
	FTimerHandle AttackResetTimer;
	GetWorldTimerManager().SetTimer(AttackResetTimer, [this]()
	{
		if (CurrentState == EAnansiCharacterState::Attacking)
		{
			SetCharacterState(EAnansiCharacterState::Idle);
		}
	}, 0.4f, false);
}

void AAnansiCharacter::Input_HeavyAttack(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	// Ground slam if airborne
	if (GetCharacterMovement()->IsFalling())
	{
		Input_GroundSlam(Value);
		return;
	}

	if (CombatComponent)
	{
		CombatComponent->RequestHeavyAttack();
		SetCharacterState(EAnansiCharacterState::Attacking);
	}

	// Heavy attack deals more damage, wider radius + aerial launch.
	if (MeleeDamage)
	{
		MeleeDamage->AttackRadius = 90.0f;
		MeleeDamage->KnockbackForce = 600.0f; // Stronger knockback for heavy
		const int32 Hits = MeleeDamage->FireAttack(30.0f);
		MeleeDamage->AttackRadius = 60.0f;
		MeleeDamage->KnockbackForce = 400.0f;

		if (Hits > 0)
		{
			// Launch enemies upward for aerial juggle opportunity
			TArray<AActor*> Enemies;
			UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);
			for (AActor* Enemy : Enemies)
			{
				if (!Enemy) continue;
				if (FVector::Dist(GetActorLocation(), Enemy->GetActorLocation()) < 200.0f)
				{
					if (ACharacter* EnemyChar = Cast<ACharacter>(Enemy))
					{
						EnemyChar->LaunchCharacter(FVector(0, 0, 600), false, true);
					}
				}
			}

			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				PC->ClientStartCameraShake(UAnansiDamageShake::StaticClass());
			}
		}
	}

	FTimerHandle AttackResetTimer;
	GetWorldTimerManager().SetTimer(AttackResetTimer, [this]()
	{
		if (CurrentState == EAnansiCharacterState::Attacking)
		{
			SetCharacterState(EAnansiCharacterState::Idle);
		}
	}, 0.6f, false);
}

void AAnansiCharacter::Input_Dodge(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	const float StaminaCost = 20.0f;
	if (!ConsumeStamina(StaminaCost))
	{
		return;
	}

	if (CombatComponent)
	{
		CombatComponent->StartDodge();
		SetCharacterState(EAnansiCharacterState::Dodging);

		// Flash player mesh to indicate i-frames
		if (PlayerVisualMesh)
		{
			PlayerVisualMesh->SetVisibility(false);
			FTimerHandle FlashTimer;
			GetWorldTimerManager().SetTimer(FlashTimer, [this]()
			{
				if (PlayerVisualMesh) PlayerVisualMesh->SetVisibility(true);
			}, 0.08f, false);

			FTimerHandle Flash2;
			GetWorldTimerManager().SetTimer(Flash2, [this]()
			{
				if (PlayerVisualMesh) PlayerVisualMesh->SetVisibility(false);
				FTimerHandle Flash3;
				GetWorldTimerManager().SetTimer(Flash3, [this]()
				{
					if (PlayerVisualMesh) PlayerVisualMesh->SetVisibility(true);
				}, 0.08f, false);
			}, 0.16f, false);
		}

		// Launch in movement direction (or backward if stationary)
		FVector DodgeDir = GetLastMovementInputVector();
		if (DodgeDir.IsNearlyZero())
		{
			DodgeDir = -GetActorForwardVector();
		}
		LaunchCharacter(DodgeDir.GetSafeNormal() * 800.0f + FVector(0, 0, 150.0f), true, true);

		// Return to idle after dodge
		FTimerHandle DodgeResetTimer;
		GetWorldTimerManager().SetTimer(DodgeResetTimer, [this]()
		{
			if (CurrentState == EAnansiCharacterState::Dodging)
			{
				SetCharacterState(EAnansiCharacterState::Idle);
			}
		}, 0.5f, false);
	}
}

void AAnansiCharacter::Input_Parry(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	if (CombatComponent)
	{
		CombatComponent->StartParry();
		SetCharacterState(EAnansiCharacterState::Parrying);
	}
}

void AAnansiCharacter::Input_CrouchToggle(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	bIsCrouching = !bIsCrouching;

	if (bIsCrouching)
	{
		Crouch();
		SetCharacterState(EAnansiCharacterState::Crouching);
	}
	else
	{
		UnCrouch();
		SetCharacterState(EAnansiCharacterState::Idle);
	}
}

void AAnansiCharacter::Input_SprintStart(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	if (bIsCrouching || CurrentStamina <= 0.0f)
	{
		return;
	}

	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AAnansiCharacter::Input_SprintStop(const FInputActionValue& Value)
{
	if (bIsSprinting)
	{
		bIsSprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	}
}

void AAnansiCharacter::Input_Interact(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	if (InteractionComponent)
	{
		InteractionComponent->Interact();
	}
}

void AAnansiCharacter::Input_SpiderSense(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
		return;

	const float EnergyCost = 25.0f;
	if (!ConsumeWebEnergy(EnergyCost))
		return;

	SetCharacterState(EAnansiCharacterState::UsingAbility);

	// Slow motion
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.3f);

	// Highlight all enemies and interactables in range
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);
	for (AActor* Enemy : Enemies)
	{
		if (FVector::Dist(GetActorLocation(), Enemy->GetActorLocation()) > SpiderSenseRadius) continue;
		if (UPrimitiveComponent* Prim = Enemy->FindComponentByClass<UPrimitiveComponent>())
		{
			Prim->SetRenderCustomDepth(true);
			Prim->SetCustomDepthStencilValue(1);
		}
	}

	TArray<AActor*> Collectibles;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Collectible"), Collectibles);
	for (AActor* Coll : Collectibles)
	{
		if (FVector::Dist(GetActorLocation(), Coll->GetActorLocation()) > SpiderSenseRadius) continue;
		if (UPrimitiveComponent* Prim = Coll->FindComponentByClass<UPrimitiveComponent>())
		{
			Prim->SetRenderCustomDepth(true);
			Prim->SetCustomDepthStencilValue(3);
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("Spider Sense activated! %d enemies, %d collectibles highlighted"),
		Enemies.Num(), Collectibles.Num());

	// End after duration
	FTimerHandle SenseTimer;
	GetWorldTimerManager().SetTimer(SenseTimer, [this, Enemies, Collectibles]()
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

		for (AActor* Enemy : Enemies)
		{
			if (!IsValid(Enemy)) continue;
			if (UPrimitiveComponent* Prim = Enemy->FindComponentByClass<UPrimitiveComponent>())
			{
				Prim->SetRenderCustomDepth(false);
			}
		}
		for (AActor* Coll : Collectibles)
		{
			if (!IsValid(Coll)) continue;
			if (UPrimitiveComponent* Prim = Coll->FindComponentByClass<UPrimitiveComponent>())
			{
				Prim->SetRenderCustomDepth(false);
			}
		}

		if (CurrentState == EAnansiCharacterState::UsingAbility)
			SetCharacterState(EAnansiCharacterState::Idle);
	}, 0.5f, false); // Real-time: 0.5s, perceived: ~1.7s due to slow-mo
}

void AAnansiCharacter::Input_SilkenPath(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
		return;

	const float EnergyCost = 20.0f;
	if (!ConsumeWebEnergy(EnergyCost))
		return;

	const FVector Forward = GetActorForwardVector();
	const FVector Origin = GetActorLocation();

	// Check for grapple points first
	TArray<AActor*> GrapplePoints;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("GrapplePoint"), GrapplePoints);

	for (AActor* GP : GrapplePoints)
	{
		if (!GP) continue;
		const float Dist = FVector::Dist(Origin, GP->GetActorLocation());
		if (Dist > 2000.0f || Dist < 200.0f) continue;

		const FVector Dir = (GP->GetActorLocation() - Origin).GetSafeNormal();
		if (FVector::DotProduct(Forward, Dir) < 0.5f) continue;

		// Zip to grapple point
		const FVector ZipDir = (GP->GetActorLocation() - Origin).GetSafeNormal();
		LaunchCharacter(ZipDir * FMath::Min(Dist * 2.5f, 3000.0f) + FVector(0, 0, 200), true, true);

#if ENABLE_DRAW_DEBUG
		DrawDebugLine(GetWorld(), Origin, GP->GetActorLocation(), FColor::Green, false, 0.5f, 0, 3.0f);
#endif
		UE_LOG(LogAnansi, Log, TEXT("Silken Path: Grapple zip to %s!"), *GP->GetName());
		return;
	}

	// Find nearest enemy in front of us
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;

	for (AActor* Enemy : Enemies)
	{
		if (!Enemy) continue;
		const float Dist = FVector::Dist(Origin, Enemy->GetActorLocation());
		if (Dist > 1500.0f) continue;

		const FVector Dir = (Enemy->GetActorLocation() - Origin).GetSafeNormal();
		const float Dot = FVector::DotProduct(Forward, Dir);
		if (Dot < 0.3f) continue; // Must be roughly in front

		const float Score = Dot * (1.0f - Dist / 1500.0f);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Enemy;
		}
	}

	if (BestTarget)
	{
		// Pull target towards us
		const FVector PullDir = (Origin - BestTarget->GetActorLocation()).GetSafeNormal();
		if (ACharacter* TargetChar = Cast<ACharacter>(BestTarget))
		{
			TargetChar->LaunchCharacter(PullDir * 1200.0f + FVector(0, 0, 200), true, true);
		}

		// Draw web line briefly
#if ENABLE_DRAW_DEBUG
		DrawDebugLine(GetWorld(), Origin, BestTarget->GetActorLocation(),
			FColor::White, false, 0.5f, 0, 3.0f);
#endif

		UE_LOG(LogAnansi, Log, TEXT("Silken Path: Pulled %s!"), *BestTarget->GetName());
	}
	else
	{
		// No enemy target — web-zip forward (traversal dash)
		FHitResult ZipHit;
		FCollisionQueryParams ZipParams;
		ZipParams.AddIgnoredActor(this);

		const FVector ZipEnd = Origin + Forward * 1500.0f;
		FVector ZipTarget = ZipEnd;

		if (GetWorld()->LineTraceSingleByChannel(ZipHit, Origin, ZipEnd, ECC_WorldStatic, ZipParams))
		{
			// Stop short of the wall
			ZipTarget = ZipHit.ImpactPoint - Forward * 100.0f;
		}

		const float ZipDist = FVector::Dist(Origin, ZipTarget);
		if (ZipDist > 200.0f)
		{
			// Launch toward target
			const FVector ZipDir = (ZipTarget - Origin).GetSafeNormal();
			LaunchCharacter(ZipDir * FMath::Min(ZipDist * 2.0f, 2000.0f) + FVector(0, 0, 100), true, true);

#if ENABLE_DRAW_DEBUG
			DrawDebugLine(GetWorld(), Origin, ZipTarget, FColor::Cyan, false, 0.3f, 0, 3.0f);
#endif
			UE_LOG(LogAnansi, Log, TEXT("Silken Path: Web-zip forward (%.0f units)"), ZipDist);
		}
	}
}

void AAnansiCharacter::Input_GroundSlam(const FInputActionValue& Value)
{
	if (!GetCharacterMovement()->IsFalling()) return;

	SetCharacterState(EAnansiCharacterState::Attacking);

	// Slam downward fast
	GetCharacterMovement()->Velocity = FVector(0, 0, -2500.0f);

	// When we land, deal AoE damage — use a timer to check for landing
	FTimerHandle SlamCheckTimer;
	GetWorldTimerManager().SetTimer(SlamCheckTimer, [this]()
	{
		if (!GetCharacterMovement()->IsFalling())
		{
			// Landed — AoE damage around landing point
			const FVector LandPoint = GetActorLocation();
			const float SlamRadius = 400.0f;
			const float SlamDamage = 40.0f;

			TArray<AActor*> Enemies;
			UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);
			for (AActor* Enemy : Enemies)
			{
				if (!Enemy) continue;
				const float Dist = FVector::Dist(LandPoint, Enemy->GetActorLocation());
				if (Dist > SlamRadius) continue;

				const float DamageFalloff = 1.0f - (Dist / SlamRadius);
				FDamageEvent DmgEvent;
				Enemy->TakeDamage(SlamDamage * DamageFalloff, DmgEvent, GetController(), this);

				// Launch enemies away
				if (ACharacter* EChar = Cast<ACharacter>(Enemy))
				{
					const FVector PushDir = (Enemy->GetActorLocation() - LandPoint).GetSafeNormal();
					EChar->LaunchCharacter(PushDir * 800.0f + FVector(0, 0, 300), true, true);
				}
			}

#if ENABLE_DRAW_DEBUG
			DrawDebugSphere(GetWorld(), LandPoint, SlamRadius, 16, FColor::Orange, false, 0.5f);
#endif

			// Camera shake
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				PC->ClientStartCameraShake(UAnansiDamageShake::StaticClass());
			}

			if (CurrentState == EAnansiCharacterState::Attacking)
				SetCharacterState(EAnansiCharacterState::Idle);

			UE_LOG(LogAnansi, Log, TEXT("GROUND SLAM! AoE damage at landing"));
		}
	}, 0.05f, true); // Check every frame until we land

	// Safety — stop checking after 3 seconds
	FTimerHandle SlamTimeoutTimer;
	FTimerHandle SlamCheckCopy = SlamCheckTimer;
	GetWorldTimerManager().SetTimer(SlamTimeoutTimer, [this, SlamCheckCopy]() mutable
	{
		GetWorldTimerManager().ClearTimer(SlamCheckCopy);
		if (CurrentState == EAnansiCharacterState::Attacking)
			SetCharacterState(EAnansiCharacterState::Idle);
	}, 3.0f, false);
}

void AAnansiCharacter::Input_TrickMirror(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
		return;

	// If decoy exists, detonate it
	if (ActiveDecoy.IsValid())
	{
		const FVector DecoyLoc = ActiveDecoy->GetActorLocation();

		// Explosion damage to nearby enemies
		TArray<AActor*> Enemies;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);
		for (AActor* Enemy : Enemies)
		{
			if (!Enemy) continue;
			if (FVector::Dist(DecoyLoc, Enemy->GetActorLocation()) < 400.0f)
			{
				FDamageEvent DamageEvent;
				Enemy->TakeDamage(40.0f, DamageEvent, GetController(), this);
			}
		}

#if ENABLE_DRAW_DEBUG
		DrawDebugSphere(GetWorld(), DecoyLoc, 400.0f, 16, FColor::Purple, false, 0.5f);
#endif

		ActiveDecoy->Destroy();
		ActiveDecoy.Reset();
		UE_LOG(LogAnansi, Log, TEXT("Trick Mirror: Decoy detonated! AoE damage applied."));
		return;
	}

	// Spawn a new decoy
	const float EnergyCost = 30.0f;
	if (!ConsumeWebEnergy(EnergyCost))
		return;

	const FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 300.0f;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Decoy = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLoc, GetActorRotation(), Params);
	if (Decoy)
	{
		// Add a visible mesh to the decoy
		UStaticMeshComponent* DecoyMesh = NewObject<UStaticMeshComponent>(Decoy, TEXT("DecoyMesh"));
		UStaticMesh* CylMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		if (CylMesh)
		{
			DecoyMesh->SetStaticMesh(CylMesh);
			DecoyMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.0f));
			DecoyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		Decoy->SetRootComponent(DecoyMesh);
		DecoyMesh->RegisterComponent();

		Decoy->Tags.Add(FName("Enemy")); // Enemies will target it
		Decoy->SetLifeSpan(10.0f);
		ActiveDecoy = Decoy;

		// Redirect nearby enemy AI to target the decoy
		TArray<AActor*> Enemies;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);
		for (AActor* Enemy : Enemies)
		{
			if (!Enemy || Enemy == Decoy) continue;
			if (FVector::Dist(SpawnLoc, Enemy->GetActorLocation()) < 1500.0f)
			{
				if (USimpleEnemyAI* AI = Enemy->FindComponentByClass<USimpleEnemyAI>())
				{
					AI->AlertToThreat(Decoy);
				}
			}
		}

		UE_LOG(LogAnansi, Log, TEXT("Trick Mirror: Decoy spawned! Press V again to detonate."));
	}
}

void AAnansiCharacter::Input_LockOn(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->ToggleLockOn();
	}
}

void AAnansiCharacter::Input_WebSwing(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	// Require airborne to swing.
	if (!GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// Need web energy.
	if (CurrentWebEnergy <= 0.0f)
	{
		return;
	}

	// Find nearest WebSwingAnchor, or fall back to upward trace.
	const FVector PlayerLoc = GetActorLocation();
	bool bFoundAnchor = false;

	// Search for placed web anchors first
	TArray<AActor*> Anchors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("WebAnchor"), Anchors);

	float BestDist = MAX_FLT;
	FVector BestAnchor = FVector::ZeroVector;

	for (AActor* Anchor : Anchors)
	{
		if (!Anchor) continue;
		const float Dist = FVector::Dist(PlayerLoc, Anchor->GetActorLocation());
		// Must be above us and within range
		if (Anchor->GetActorLocation().Z > PlayerLoc.Z + 100.0f && Dist < 2000.0f && Dist < BestDist)
		{
			BestDist = Dist;
			BestAnchor = Anchor->GetActorLocation();
			bFoundAnchor = true;
		}
	}

	// Fallback: trace upward for ceilings/overhangs
	if (!bFoundAnchor)
	{
		FHitResult Hit;
		const FVector End = PlayerLoc + FVector(0.0f, 0.0f, 2000.0f);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, PlayerLoc, End, ECC_WorldStatic, Params))
		{
			BestAnchor = Hit.ImpactPoint;
			bFoundAnchor = true;
		}
	}

	if (bFoundAnchor)
	{
		WebAnchorPoint = BestAnchor;
		bIsWebSwinging = true;
		SetCharacterState(EAnansiCharacterState::WebSwinging);
		GetCharacterMovement()->GravityScale = 0.0f;
	}
}

void AAnansiCharacter::Input_WebSwingRelease(const FInputActionValue& Value)
{
	if (bIsWebSwinging)
	{
		bIsWebSwinging = false;
		GetCharacterMovement()->GravityScale = 1.0f;

		// Give a momentum boost on release.
		const FVector LaunchVelocity = GetVelocity() * 1.2f + FVector(0.0f, 0.0f, 300.0f);
		LaunchCharacter(LaunchVelocity, true, true);

		SetCharacterState(EAnansiCharacterState::Jumping);
	}
}

// ---------------------------------------------------------------------------
// State machine
// ---------------------------------------------------------------------------

void AAnansiCharacter::SetCharacterState(EAnansiCharacterState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	// Prevent transitions out of Dead unless explicitly resurrected.
	if (CurrentState == EAnansiCharacterState::Dead && NewState != EAnansiCharacterState::Idle)
	{
		return;
	}

	const EAnansiCharacterState OldState = CurrentState;
	CurrentState = NewState;
	OnCharacterStateChanged.Broadcast(OldState, NewState);
}

void AAnansiCharacter::UpdateStateFromMovement(float DeltaTime)
{
	// Don't auto-update state during certain override states.
	if (CurrentState == EAnansiCharacterState::Dead ||
		CurrentState == EAnansiCharacterState::Stunned ||
		CurrentState == EAnansiCharacterState::Attacking ||
		CurrentState == EAnansiCharacterState::Dodging ||
		CurrentState == EAnansiCharacterState::Parrying ||
		CurrentState == EAnansiCharacterState::UsingAbility ||
		CurrentState == EAnansiCharacterState::WallRunning ||
		CurrentState == EAnansiCharacterState::WebSwinging)
	{
		return;
	}

	const UCharacterMovementComponent* CMC = GetCharacterMovement();

	if (CMC->IsFalling())
	{
		SetCharacterState(EAnansiCharacterState::Jumping);
	}
	else if (CMC->IsCrouching())
	{
		SetCharacterState(EAnansiCharacterState::Crouching);
	}
	else if (CMC->Velocity.Size2D() > 10.0f)
	{
		SetCharacterState(EAnansiCharacterState::Moving);
	}
	else
	{
		SetCharacterState(EAnansiCharacterState::Idle);
	}
}

// ---------------------------------------------------------------------------
// Wall-run
// ---------------------------------------------------------------------------

bool AAnansiCharacter::TryStartWallRun()
{
	const FVector Velocity2D = FVector(GetVelocity().X, GetVelocity().Y, 0.0f);
	if (Velocity2D.Size() < WallRunMinSpeed)
	{
		return false;
	}

	// Trace left and right for a wall surface.
	const FVector Location = GetActorLocation();
	const FVector Right = GetActorRightVector();

	FHitResult HitRight, HitLeft;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const float TraceLength = 80.0f;

	bool bFoundWall = false;

	if (GetWorld()->LineTraceSingleByChannel(HitRight, Location, Location + Right * TraceLength, ECC_WorldStatic, Params))
	{
		WallRunNormal = HitRight.ImpactNormal;
		bFoundWall = true;
	}
	else if (GetWorld()->LineTraceSingleByChannel(HitLeft, Location, Location - Right * TraceLength, ECC_WorldStatic, Params))
	{
		WallRunNormal = HitLeft.ImpactNormal;
		bFoundWall = true;
	}

	if (bFoundWall)
	{
		WallRunTimer = 0.0f;
		GetCharacterMovement()->GravityScale = 0.0f;
		GetCharacterMovement()->Velocity.Z = 0.0f;
		SetCharacterState(EAnansiCharacterState::WallRunning);
		return true;
	}

	return false;
}

void AAnansiCharacter::TickWallRun(float DeltaTime)
{
	WallRunTimer += DeltaTime;

	if (WallRunTimer >= WallRunMaxDuration)
	{
		// Duration expired — detach and jump off.
		GetCharacterMovement()->GravityScale = 1.0f;
		LaunchCharacter(WallRunNormal * 400.0f + FVector(0.0f, 0.0f, 400.0f), true, true);
		SetCharacterState(EAnansiCharacterState::Jumping);
		return;
	}

	// Keep the character pressed against the wall.
	AddMovementInput(-WallRunNormal, 1.0f);

	// Verify wall is still there.
	FHitResult WallCheck;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (!GetWorld()->LineTraceSingleByChannel(WallCheck, GetActorLocation(),
		GetActorLocation() - WallRunNormal * 80.0f, ECC_WorldStatic, Params))
	{
		// Wall ended — jump off.
		GetCharacterMovement()->GravityScale = 1.0f;
		LaunchCharacter(WallRunNormal * 300.0f + FVector(0.0f, 0.0f, 300.0f), true, true);
		SetCharacterState(EAnansiCharacterState::Jumping);
	}
}

// ---------------------------------------------------------------------------
// Web-swing
// ---------------------------------------------------------------------------

void AAnansiCharacter::TickWebSwing(float DeltaTime)
{
	if (!bIsWebSwinging)
	{
		return;
	}

	// Consume web energy.
	if (!ConsumeWebEnergy(WebSwingEnergyCostPerSecond * DeltaTime))
	{
		// Out of energy — release swing.
		Input_WebSwingRelease(FInputActionValue());
		return;
	}

	// Simple pendulum: accelerate the character towards the nadir of the arc.
	const FVector ToAnchor = WebAnchorPoint - GetActorLocation();
	const float RopeLength = ToAnchor.Size();
	const FVector SwingDir = ToAnchor.GetSafeNormal();

	// Gravity-like pull towards the anchor's vertical.
	const float SwingForce = 980.0f; // approximate gravity
	const FVector Acceleration = FVector(SwingDir.X, SwingDir.Y, 0.0f).GetSafeNormal() * SwingForce;

	FVector NewVelocity = GetVelocity() + Acceleration * DeltaTime;

	// Constrain to the rope length (keep distance to anchor constant).
	const FVector NewPosition = GetActorLocation() + NewVelocity * DeltaTime;
	const FVector ConstrainedOffset = (NewPosition - WebAnchorPoint).GetSafeNormal() * RopeLength;
	const FVector ConstrainedPosition = WebAnchorPoint + ConstrainedOffset;
	const FVector CorrectedVelocity = (ConstrainedPosition - GetActorLocation()) / DeltaTime;

	GetCharacterMovement()->Velocity = CorrectedVelocity;

	// Draw web rope
#if ENABLE_DRAW_DEBUG
	DrawDebugLine(GetWorld(), GetActorLocation(), WebAnchorPoint,
		FColor::White, false, -1.0f, 0, 2.0f);
	DrawDebugSphere(GetWorld(), WebAnchorPoint, 15.0f, 8, FColor::Cyan, false, -1.0f, 0, 1.0f);
#endif
}

// ---------------------------------------------------------------------------
// Health
// ---------------------------------------------------------------------------

void AAnansiCharacter::ApplyDamage(float Amount)
{
	if (!IsAlive() || Amount <= 0.0f)
	{
		return;
	}

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Max(CurrentHealth - Amount, 0.0f);
	OnHealthChanged.Broadcast(OldHealth, CurrentHealth);

	if (CurrentHealth <= 0.0f)
	{
		SetCharacterState(EAnansiCharacterState::Dead);
		UE_LOG(LogAnansi, Log, TEXT("Anansi has fallen."));

		// Show game over screen
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (AAnansiDevHUD* DevHUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
			{
				DevHUD->ShowGameOver();
			}
		}
	}
}

void AAnansiCharacter::ApplyHealing(float Amount)
{
	if (!IsAlive() || Amount <= 0.0f)
	{
		return;
	}

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
	OnHealthChanged.Broadcast(OldHealth, CurrentHealth);
}

// ---------------------------------------------------------------------------
// Respawn
// ---------------------------------------------------------------------------

void AAnansiCharacter::Respawn()
{
	// Restore health
	CurrentHealth = MaxHealth;
	CurrentWebEnergy = MaxWebEnergy;
	CurrentStamina = MaxStamina;
	OnHealthChanged.Broadcast(0.0f, CurrentHealth);
	OnWebEnergyChanged.Broadcast(0.0f, CurrentWebEnergy);

	// Reset state
	SetCharacterState(EAnansiCharacterState::Idle);
	bIsSprinting = false;
	bIsCrouching = false;
	bIsWebSwinging = false;
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// Move to checkpoint or spawn point
	ACheckpoint* CP = ACheckpoint::GetActiveCheckpoint(this);
	if (CP)
	{
		SetActorLocation(CP->GetRespawnLocation());
		SetActorRotation(CP->GetActorRotation());
	}
	else if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (AActor* PlayerStart = GetWorld()->GetAuthGameMode()->FindPlayerStart(PC))
		{
			SetActorLocation(PlayerStart->GetActorLocation() + FVector(0, 0, 100));
			SetActorRotation(PlayerStart->GetActorRotation());
		}
	}

	// Enable collision back
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Reset game over on HUD
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (AAnansiDevHUD* DevHUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			DevHUD->SetPauseVisible(false);
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("Player respawned."));
}

// ---------------------------------------------------------------------------
// TakeDamage override
// ---------------------------------------------------------------------------

float AAnansiCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!IsAlive() || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	// Check dodge i-frames
	if (CombatComponent && CombatComponent->IsInvulnerable())
	{
		return 0.0f;
	}

	// Check parry
	if (CombatComponent && CombatComponent->IsParrying())
	{
		// Parry success — riposte damage to attacker
		if (DamageCauser)
		{
			FDamageEvent RiposteDmg;
			DamageCauser->TakeDamage(25.0f, RiposteDmg, GetController(), this);

			// Stagger the attacker
			if (ACharacter* AttackerChar = Cast<ACharacter>(DamageCauser))
			{
				const FVector KnockDir = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				AttackerChar->LaunchCharacter(KnockDir * 500.0f, true, false);
			}

			// Slow-mo flash
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.15f);
			FTimerHandle ParrySlowMo;
			GetWorldTimerManager().SetTimer(ParrySlowMo, [this]()
			{
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
			}, 0.03f, false);

			// Camera shake
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				PC->ClientStartCameraShake(UAnansiHitShake::StaticClass());
				if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
				{
					HUD->AddDamageNumber(25.0f, DamageCauser->GetActorLocation() + FVector(0, 0, 120), true);
				}
			}

			if (UGameInstance* GI = GetWorld()->GetGameInstance())
			{
				if (UCombatStatsSubsystem* Stats = GI->GetSubsystem<UCombatStatsSubsystem>())
					Stats->RecordParry();
			}
		}

		UE_LOG(LogAnansi, Log, TEXT("PARRY RIPOSTE! 25 damage to %s"),
			DamageCauser ? *DamageCauser->GetName() : TEXT("unknown"));
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	ApplyDamage(ActualDamage);

	// Camera shake on damage
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		PC->ClientStartCameraShake(UAnansiDamageShake::StaticClass());

		// Hit direction indicator
		if (AAnansiDevHUD* DevHUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			const FVector SourceLoc = DamageCauser ? DamageCauser->GetActorLocation() : GetActorLocation();
			DevHUD->ShowHitDirection(SourceLoc);
			DevHUD->AddDamageNumber(ActualDamage, GetActorLocation() + FVector(0, 0, 100), ActualDamage >= 25.0f);
		}
	}

	// Reset combo on taking damage
	if (CombatComponent)
	{
		CombatComponent->ResetCombo();
	}

	UE_LOG(LogAnansi, Log, TEXT("Player took %.1f damage (%.1f / %.1f HP)"),
		ActualDamage, CurrentHealth, MaxHealth);

	return ActualDamage;
}

// ---------------------------------------------------------------------------
// Web Energy
// ---------------------------------------------------------------------------

bool AAnansiCharacter::ConsumeWebEnergy(float Amount)
{
	if (Amount <= 0.0f)
	{
		return true;
	}

	if (CurrentWebEnergy < Amount)
	{
		return false;
	}

	const float OldEnergy = CurrentWebEnergy;
	CurrentWebEnergy -= Amount;
	OnWebEnergyChanged.Broadcast(OldEnergy, CurrentWebEnergy);
	return true;
}

void AAnansiCharacter::RestoreWebEnergy(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	const float OldEnergy = CurrentWebEnergy;
	CurrentWebEnergy = FMath::Min(CurrentWebEnergy + Amount, MaxWebEnergy);
	OnWebEnergyChanged.Broadcast(OldEnergy, CurrentWebEnergy);
}

// ---------------------------------------------------------------------------
// Stamina
// ---------------------------------------------------------------------------

bool AAnansiCharacter::ConsumeStamina(float Amount)
{
	if (Amount <= 0.0f)
	{
		return true;
	}

	if (CurrentStamina < Amount)
	{
		return false;
	}

	const float OldStamina = CurrentStamina;
	CurrentStamina -= Amount;
	StaminaRegenCooldown = StaminaRegenDelay;
	OnStaminaChanged.Broadcast(OldStamina, CurrentStamina);
	return true;
}

// ---------------------------------------------------------------------------
// Combo
// ---------------------------------------------------------------------------

int32 AAnansiCharacter::GetComboCount() const
{
	return CombatComponent ? CombatComponent->GetComboCount() : 0;
}

// ---------------------------------------------------------------------------
// Spider Sense
// ---------------------------------------------------------------------------

void AAnansiCharacter::OnSpiderSenseBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// Only trigger on actors tagged as threats (enemies, projectiles).
	if (OtherActor->ActorHasTag(FName("Enemy")) || OtherActor->ActorHasTag(FName("Projectile")))
	{
		OnSpiderSenseTriggered.Broadcast(OtherActor);
		UE_LOG(LogAnansi, Verbose, TEXT("Spider Sense triggered by %s"), *OtherActor->GetName());
	}
}

// ---------------------------------------------------------------------------
// GAS
// ---------------------------------------------------------------------------

void AAnansiCharacter::GrantInitialAbilities()
{
	if (!AbilitySystemComp)
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : InitialAbilities)
	{
		if (AbilityClass)
		{
			AbilitySystemComp->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
		}
	}
}

// ---------------------------------------------------------------------------
// Auto-load Input Assets
// ---------------------------------------------------------------------------

void AAnansiCharacter::AutoLoadInputAssets()
{
	// Helper: try loading from disk first, create in-memory if missing.
	auto GetOrCreateAction = [](TObjectPtr<UInputAction>& Ptr, const TCHAR* DiskPath, const TCHAR* Name, EInputActionValueType ValueType)
	{
		if (Ptr) return;

		// Try disk first
		Ptr = LoadObject<UInputAction>(nullptr, DiskPath);
		if (Ptr) return;

		// Create transient in-memory action
		Ptr = NewObject<UInputAction>(GetTransientPackage(), Name);
		Ptr->ValueType = ValueType;
		Ptr->bConsumeInput = true;
		Ptr->AddToRoot(); // Prevent GC
	};

	GetOrCreateAction(IA_Move,        TEXT("/Game/Input/IA_Move.IA_Move"),               TEXT("IA_Move"),        EInputActionValueType::Axis2D);
	GetOrCreateAction(IA_Look,        TEXT("/Game/Input/IA_Look.IA_Look"),               TEXT("IA_Look"),        EInputActionValueType::Axis2D);
	GetOrCreateAction(IA_Jump,        TEXT("/Game/Input/IA_Jump.IA_Jump"),               TEXT("IA_Jump"),        EInputActionValueType::Boolean);
	GetOrCreateAction(IA_LightAttack, TEXT("/Game/Input/IA_LightAttack.IA_LightAttack"), TEXT("IA_LightAttack"), EInputActionValueType::Boolean);
	GetOrCreateAction(IA_HeavyAttack, TEXT("/Game/Input/IA_HeavyAttack.IA_HeavyAttack"), TEXT("IA_HeavyAttack"), EInputActionValueType::Boolean);
	GetOrCreateAction(IA_Dodge,       TEXT("/Game/Input/IA_Dodge.IA_Dodge"),             TEXT("IA_Dodge"),       EInputActionValueType::Boolean);
	GetOrCreateAction(IA_Parry,       TEXT("/Game/Input/IA_Parry.IA_Parry"),             TEXT("IA_Parry"),       EInputActionValueType::Boolean);
	GetOrCreateAction(IA_Crouch,      TEXT("/Game/Input/IA_Crouch.IA_Crouch"),           TEXT("IA_Crouch"),      EInputActionValueType::Boolean);
	GetOrCreateAction(IA_LockOn,      TEXT("/Game/Input/IA_LockOn.IA_LockOn"),           TEXT("IA_LockOn"),      EInputActionValueType::Boolean);
	GetOrCreateAction(IA_WebSwing,    TEXT("/Game/Input/IA_WebSwing.IA_WebSwing"),       TEXT("IA_WebSwing"),    EInputActionValueType::Boolean);
	GetOrCreateAction(IA_Sprint,      TEXT("/Game/Input/IA_Sprint.IA_Sprint"),           TEXT("IA_Sprint"),      EInputActionValueType::Boolean);
	GetOrCreateAction(IA_Interact,    TEXT("/Game/Input/IA_Interact.IA_Interact"),       TEXT("IA_Interact"),    EInputActionValueType::Boolean);
	GetOrCreateAction(IA_SpiderSense, TEXT("/Game/Input/IA_SpiderSense.IA_SpiderSense"), TEXT("IA_SpiderSense"), EInputActionValueType::Boolean);
	GetOrCreateAction(IA_SilkenPath,  TEXT("/Game/Input/IA_SilkenPath.IA_SilkenPath"),   TEXT("IA_SilkenPath"),  EInputActionValueType::Boolean);
	GetOrCreateAction(IA_TrickMirror, TEXT("/Game/Input/IA_TrickMirror.IA_TrickMirror"), TEXT("IA_TrickMirror"), EInputActionValueType::Boolean);

	// Create mapping context if missing
	if (!DefaultMappingContext)
	{
		DefaultMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_Default.IMC_Default"));
	}

	if (!DefaultMappingContext)
	{
		DefaultMappingContext = NewObject<UInputMappingContext>(GetTransientPackage(), TEXT("IMC_Default"));
		DefaultMappingContext->AddToRoot();

		// Helper to add a key mapping with optional modifiers
		auto Map = [this](UInputAction* Action, FKey Key, bool bNegate = false, bool bSwizzle = false)
		{
			if (!Action) return;
			FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(Action, Key);
			if (bNegate)
			{
				M.Modifiers.Add(NewObject<UInputModifierNegate>(DefaultMappingContext));
			}
			if (bSwizzle)
			{
				UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(DefaultMappingContext);
				Swizzle->Order = EInputAxisSwizzle::YXZ;
				M.Modifiers.Add(Swizzle);
			}
		};

		// Keyboard + Mouse
		Map(IA_Move, EKeys::W);
		Map(IA_Move, EKeys::S, true);
		Map(IA_Move, EKeys::D, false, true);
		Map(IA_Move, EKeys::A, true, true);
		Map(IA_Look, EKeys::Mouse2D);
		Map(IA_Jump,        EKeys::SpaceBar);
		Map(IA_LightAttack, EKeys::LeftMouseButton);
		Map(IA_HeavyAttack, EKeys::RightMouseButton);
		Map(IA_Dodge,       EKeys::LeftAlt);
		Map(IA_Parry,       EKeys::Q);
		Map(IA_Crouch,      EKeys::LeftControl);
		Map(IA_LockOn,      EKeys::MiddleMouseButton);
		Map(IA_WebSwing,    EKeys::E);
		Map(IA_Sprint,      EKeys::LeftShift);
		Map(IA_Interact,    EKeys::F);
		Map(IA_SpiderSense, EKeys::R);
		Map(IA_SilkenPath,  EKeys::T);
		Map(IA_TrickMirror, EKeys::V);

		// Gamepad
		Map(IA_Move, EKeys::Gamepad_Left2D);
		Map(IA_Look, EKeys::Gamepad_Right2D);
		Map(IA_Jump,        EKeys::Gamepad_FaceButton_Bottom);
		Map(IA_LightAttack, EKeys::Gamepad_FaceButton_Right);
		Map(IA_HeavyAttack, EKeys::Gamepad_FaceButton_Top);
		Map(IA_Dodge,       EKeys::Gamepad_FaceButton_Left);
		Map(IA_Parry,       EKeys::Gamepad_LeftTrigger);
		Map(IA_LockOn,      EKeys::Gamepad_RightThumbstick);
		Map(IA_WebSwing,    EKeys::Gamepad_RightTrigger);
		Map(IA_Crouch,      EKeys::Gamepad_LeftThumbstick);
		Map(IA_Interact,    EKeys::Gamepad_LeftShoulder);
		Map(IA_SpiderSense, EKeys::Gamepad_RightShoulder);
		Map(IA_SilkenPath,  EKeys::Gamepad_DPad_Up);
		Map(IA_Sprint,      EKeys::Gamepad_LeftThumbstick);

		UE_LOG(LogAnansi, Log, TEXT("AutoLoadInputAssets: Created in-memory mapping context with all bindings."));
	}

	UE_LOG(LogAnansi, Log, TEXT("AutoLoadInputAssets: MappingContext=%s, Move=%s, WebSwing=%s"),
		DefaultMappingContext ? TEXT("OK") : TEXT("MISSING"),
		IA_Move ? TEXT("OK") : TEXT("MISSING"),
		IA_WebSwing ? TEXT("OK") : TEXT("MISSING"));
}

// ---------------------------------------------------------------------------
// Wall slide
// ---------------------------------------------------------------------------

void AAnansiCharacter::CheckWallSlide()
{
	// Only check while falling and not already wall-running/swinging
	if (!GetCharacterMovement()->IsFalling() || CurrentState == EAnansiCharacterState::WallRunning ||
		CurrentState == EAnansiCharacterState::WebSwinging)
	{
		if (bIsWallSliding)
		{
			bIsWallSliding = false;
			GetCharacterMovement()->GravityScale = 1.0f;
		}
		return;
	}

	// Trace left and right for walls
	const FVector Loc = GetActorLocation();
	const FVector Right = GetActorRightVector();
	const float TraceLen = 60.0f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult HitR, HitL;
	bool bTouchingWall = false;

	if (GetWorld()->LineTraceSingleByChannel(HitR, Loc, Loc + Right * TraceLen, ECC_WorldStatic, Params))
	{
		WallSlideNormal = HitR.ImpactNormal;
		bTouchingWall = true;
	}
	else if (GetWorld()->LineTraceSingleByChannel(HitL, Loc, Loc - Right * TraceLen, ECC_WorldStatic, Params))
	{
		WallSlideNormal = HitL.ImpactNormal;
		bTouchingWall = true;
	}

	if (bTouchingWall && GetVelocity().Z < -100.0f)
	{
		if (!bIsWallSliding)
		{
			bIsWallSliding = true;
		}

		// Slow fall — reduce gravity
		GetCharacterMovement()->GravityScale = 0.3f;

		// Cap downward velocity
		FVector Vel = GetVelocity();
		Vel.Z = FMath::Max(Vel.Z, -150.0f);
		GetCharacterMovement()->Velocity = Vel;

#if ENABLE_DRAW_DEBUG
		DrawDebugLine(GetWorld(), Loc, Loc - WallSlideNormal * 50.0f,
			FColor::Yellow, false, -1.0f, 0, 1.0f);
#endif
	}
	else if (bIsWallSliding)
	{
		bIsWallSliding = false;
		GetCharacterMovement()->GravityScale = 1.0f;
	}
}

// ---------------------------------------------------------------------------
// Footstep sound
// ---------------------------------------------------------------------------

void AAnansiCharacter::PlayFootstepSound()
{
	// Placeholder — in production, this traces for physical material
	// and plays the appropriate footstep sound asset.
	// For now it's silent but the timing system is in place.
	UE_LOG(LogAnansi, VeryVerbose, TEXT("Footstep at speed %.0f"), GetVelocity().Size2D());
}
