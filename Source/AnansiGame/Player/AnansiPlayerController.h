// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AnansiPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * Buffered input entry used to queue combat inputs so that actions pressed
 * slightly before the previous animation ends are not lost.
 */
USTRUCT()
struct FBufferedInput
{
	GENERATED_BODY()

	UPROPERTY()
	FName ActionName;

	/** World time when this input was buffered. */
	float BufferTime = 0.0f;
};

/**
 * AAnansiPlayerController
 *
 * Handles:
 *  - Enhanced Input setup and mapping context management.
 *  - Camera management (smooth lock-on interpolation, target switching).
 *  - Input buffering for combat responsiveness.
 *  - UI widget management (HUD, pause menu).
 *  - Pause handling.
 */
UCLASS()
class ANANSIGAME_API AAnansiPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAnansiPlayerController();

	/** Use our custom cheat manager for dev commands. */
	virtual void AddCheats(bool bForce = false) override;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	// -----------------------------------------------------------------------
	// Input buffering
	// -----------------------------------------------------------------------

	/**
	 * Buffer an input action for later consumption. If the character is in a
	 * state that cannot process the input immediately, the buffer will hold it
	 * for up to InputBufferWindow seconds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Input")
	void BufferInput(FName ActionName);

	/**
	 * Try to consume a buffered input of the given name.
	 * Returns true and removes the entry if found within the buffer window.
	 */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Input")
	bool ConsumeBufferedInput(FName ActionName);

	/** Maximum time (seconds) a buffered input remains valid. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Input")
	float InputBufferWindow = 0.2f;

	// -----------------------------------------------------------------------
	// Camera / Lock-on
	// -----------------------------------------------------------------------

	/** Smoothly interpolate the camera towards the lock-on target. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Camera")
	void UpdateLockOnCamera(float DeltaTime);

	/** Speed at which the camera rotation interpolates towards the lock-on target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Camera", meta = (ClampMin = "0.0"))
	float LockOnInterpSpeed = 8.0f;

	// -----------------------------------------------------------------------
	// UI management
	// -----------------------------------------------------------------------

	/** Show or hide the main HUD. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|UI")
	void SetHUDVisible(bool bVisible);

	// -----------------------------------------------------------------------
	// Pause
	// -----------------------------------------------------------------------

	/** Toggle game pause and show/hide the pause menu. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|UI")
	void TogglePause();

protected:
	// -- Input actions used at the controller level (pause, UI) -------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_Pause;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_SwitchTargetLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Input")
	TObjectPtr<UInputAction> IA_SwitchTargetRight;

	void Input_Pause(const FInputActionValue& Value);
	void Input_SwitchTargetLeft(const FInputActionValue& Value);
	void Input_SwitchTargetRight(const FInputActionValue& Value);

private:
	/** Active input buffer. Entries expire each tick. */
	UPROPERTY()
	TArray<FBufferedInput> InputBuffer;

	/** Expire old entries from the input buffer. */
	void CleanInputBuffer();

	bool bIsPaused = false;
};
