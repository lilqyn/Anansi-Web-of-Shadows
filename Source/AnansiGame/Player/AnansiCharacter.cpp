// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Player/AnansiCharacter.h"
#include "AnansiGame.h"
#include "Combat/CombatComponent.h"
#include "Combat/WeaponTrace.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
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

	// -- Combat components --------------------------------------------------
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	WeaponTrace = CreateDefaultSubobject<UWeaponTraceComponent>(TEXT("WeaponTrace"));

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
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void AAnansiCharacter::BeginPlay()
{
	Super::BeginPlay();

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

	Jump();
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
}

void AAnansiCharacter::Input_HeavyAttack(const FInputActionValue& Value)
{
	if (CurrentState == EAnansiCharacterState::Dead || CurrentState == EAnansiCharacterState::Stunned)
	{
		return;
	}

	if (CombatComponent)
	{
		CombatComponent->RequestHeavyAttack();
		SetCharacterState(EAnansiCharacterState::Attacking);
	}
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

	// Trace upwards to find a swing anchor.
	FHitResult Hit;
	const FVector Start = GetActorLocation();
	const FVector End = Start + FVector(0.0f, 0.0f, 2000.0f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
	{
		WebAnchorPoint = Hit.ImpactPoint;
		bIsWebSwinging = true;
		SetCharacterState(EAnansiCharacterState::WebSwinging);

		// Disable gravity while swinging — the pendulum is simulated manually.
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
