// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Player/AnansiPlayerController.h"
#include "Player/AnansiCharacter.h"
#include "Combat/CombatComponent.h"
#include "Core/AnansiCheatManager.h"
#include "AnansiGame.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

AAnansiPlayerController::AAnansiPlayerController()
{
	CheatClass = UAnansiCheatManager::StaticClass();
}

void AAnansiPlayerController::AddCheats(bool bForce)
{
	// Always add cheats in non-shipping builds for dev testing.
	Super::AddCheats(true);
}

void AAnansiPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AAnansiPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CleanInputBuffer();
	UpdateLockOnCamera(DeltaTime);
}

// ---------------------------------------------------------------------------
// Input setup
// ---------------------------------------------------------------------------

void AAnansiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		return;
	}

	if (IA_Pause)
	{
		EIC->BindAction(IA_Pause, ETriggerEvent::Started, this, &AAnansiPlayerController::Input_Pause);
	}
	if (IA_SwitchTargetLeft)
	{
		EIC->BindAction(IA_SwitchTargetLeft, ETriggerEvent::Started, this, &AAnansiPlayerController::Input_SwitchTargetLeft);
	}
	if (IA_SwitchTargetRight)
	{
		EIC->BindAction(IA_SwitchTargetRight, ETriggerEvent::Started, this, &AAnansiPlayerController::Input_SwitchTargetRight);
	}
}

// ---------------------------------------------------------------------------
// Input handlers
// ---------------------------------------------------------------------------

void AAnansiPlayerController::Input_Pause(const FInputActionValue& Value)
{
	TogglePause();
}

void AAnansiPlayerController::Input_SwitchTargetLeft(const FInputActionValue& Value)
{
	if (AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(GetPawn()))
	{
		if (UCombatComponent* Combat = Anansi->CombatComponent)
		{
			Combat->SwitchLockOnTarget(-1.0f);
		}
	}
}

void AAnansiPlayerController::Input_SwitchTargetRight(const FInputActionValue& Value)
{
	if (AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(GetPawn()))
	{
		if (UCombatComponent* Combat = Anansi->CombatComponent)
		{
			Combat->SwitchLockOnTarget(1.0f);
		}
	}
}

// ---------------------------------------------------------------------------
// Input buffering
// ---------------------------------------------------------------------------

void AAnansiPlayerController::BufferInput(FName ActionName)
{
	FBufferedInput Entry;
	Entry.ActionName = ActionName;
	Entry.BufferTime = GetWorld()->GetTimeSeconds();
	InputBuffer.Add(Entry);
}

bool AAnansiPlayerController::ConsumeBufferedInput(FName ActionName)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	for (int32 i = InputBuffer.Num() - 1; i >= 0; --i)
	{
		if (InputBuffer[i].ActionName == ActionName &&
			(CurrentTime - InputBuffer[i].BufferTime) <= InputBufferWindow)
		{
			InputBuffer.RemoveAt(i);
			return true;
		}
	}

	return false;
}

void AAnansiPlayerController::CleanInputBuffer()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	InputBuffer.RemoveAll([this, CurrentTime](const FBufferedInput& Entry)
	{
		return (CurrentTime - Entry.BufferTime) > InputBufferWindow;
	});
}

// ---------------------------------------------------------------------------
// Camera / Lock-on
// ---------------------------------------------------------------------------

void AAnansiPlayerController::UpdateLockOnCamera(float DeltaTime)
{
	AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(GetPawn());
	if (!Anansi || !Anansi->CombatComponent)
	{
		return;
	}

	AActor* Target = Anansi->CombatComponent->GetLockOnTarget();

	// Camera boom adjustments based on lock-on state
	if (Anansi->CameraBoom)
	{
		const float TargetArmLength = Target ? 350.0f : 400.0f;
		const FVector TargetOffset = Target ? FVector(0, 50, 60) : FVector(0, 0, 0);

		Anansi->CameraBoom->TargetArmLength = FMath::FInterpTo(
			Anansi->CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, 5.0f);
		Anansi->CameraBoom->SocketOffset = FMath::VInterpTo(
			Anansi->CameraBoom->SocketOffset, TargetOffset, DeltaTime, 5.0f);
	}

	if (!Target)
	{
		return;
	}

	// Smoothly rotate the controller to face the lock-on target.
	const FVector Start = Anansi->GetActorLocation();
	const FVector End = Target->GetActorLocation();
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Start, End);
	const FRotator CurrentRotation = GetControlRotation();
	const FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, LockOnInterpSpeed);

	// Yaw and slight pitch towards target.
	const float TargetPitch = FMath::Clamp(LookAtRotation.Pitch, -30.0f, 10.0f);
	const float SmoothedPitch = FMath::FInterpTo(CurrentRotation.Pitch, TargetPitch, DeltaTime, 3.0f);
	SetControlRotation(FRotator(SmoothedPitch, SmoothedRotation.Yaw, 0.0f));
}

// ---------------------------------------------------------------------------
// UI
// ---------------------------------------------------------------------------

void AAnansiPlayerController::SetHUDVisible(bool bVisible)
{
	// Placeholder — actual widget management would create/destroy the HUD
	// widget here. Keeping it as a stub so the API is established.
	UE_LOG(LogAnansi, Log, TEXT("HUD visibility set to %s"), bVisible ? TEXT("true") : TEXT("false"));
}

// ---------------------------------------------------------------------------
// Pause
// ---------------------------------------------------------------------------

void AAnansiPlayerController::TogglePause()
{
	bIsPaused = !bIsPaused;

	SetPause(bIsPaused);

	if (bIsPaused)
	{
		// Show pause menu, set input mode to UI.
		SetInputMode(FInputModeGameAndUI());
		SetShowMouseCursor(true);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
	}

	UE_LOG(LogAnansi, Log, TEXT("Game %s"), bIsPaused ? TEXT("paused") : TEXT("resumed"));
}
