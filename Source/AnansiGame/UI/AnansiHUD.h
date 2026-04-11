// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AnansiHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;

/**
 * UAnansiHUD
 *
 * Main in-game HUD widget for the vertical slice.
 * Displays health, web energy, stamina, combo counter/style rank,
 * spider sense indicator, and interaction prompts.
 *
 * Designed to be subclassed in Blueprint (WBP_AnansiHUD) where
 * UMG layout and visual polish are applied. C++ handles the data binding.
 */
UCLASS(Abstract, Blueprintable)
class ANANSIGAME_API UAnansiHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// -----------------------------------------------------------------------
	// Update functions called by the player controller or character
	// -----------------------------------------------------------------------

	/** Set health bar percent (0.0 - 1.0). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void SetHealthPercent(float Percent);

	/** Set web energy bar percent (0.0 - 1.0). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void SetWebEnergyPercent(float Percent);

	/** Set stamina bar percent (0.0 - 1.0). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void SetStaminaPercent(float Percent);

	/** Update combo counter and style rank text. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void SetComboDisplay(int32 ComboCount, const FText& StyleRankText);

	/** Show or hide the interaction prompt with a given label. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void SetInteractionPrompt(bool bVisible, const FText& PromptText = FText::GetEmpty());

	/** Flash the spider sense warning indicator. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void TriggerSpiderSenseWarning();

	/** Show or hide the lock-on reticle. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void SetLockOnReticleVisible(bool bVisible);

protected:
	// -----------------------------------------------------------------------
	// Widget bindings (bound in Blueprint via BindWidget)
	// -----------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Anansi|HUD")
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Anansi|HUD")
	TObjectPtr<UProgressBar> WebEnergyBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Anansi|HUD")
	TObjectPtr<UProgressBar> StaminaBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Anansi|HUD")
	TObjectPtr<UTextBlock> ComboCountText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Anansi|HUD")
	TObjectPtr<UTextBlock> StyleRankText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Anansi|HUD")
	TObjectPtr<UTextBlock> InteractionPromptText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Anansi|HUD")
	TObjectPtr<UImage> SpiderSenseIndicator;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Anansi|HUD")
	TObjectPtr<UImage> LockOnReticle;

private:
	/** Timer tracking spider sense indicator fade-out. */
	float SpiderSenseFlashTimer = 0.0f;

	/** Duration the spider sense indicator stays visible per trigger. */
	UPROPERTY(EditDefaultsOnly, Category = "Anansi|HUD")
	float SpiderSenseFlashDuration = 1.5f;
};
