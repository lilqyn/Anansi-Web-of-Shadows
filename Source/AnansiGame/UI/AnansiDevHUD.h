// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AnansiDevHUD.generated.h"

class AAnansiCharacter;

/**
 * AAnansiDevHUD
 *
 * Development HUD that draws health, stamina, web energy bars, combo counter,
 * style rank, and floating damage numbers using Canvas draw calls.
 * No UMG widgets or Blueprint assets required — works immediately.
 */
UCLASS()
class ANANSIGAME_API AAnansiDevHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	/** Add a floating damage number at a world location. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void AddDamageNumber(float Damage, FVector WorldLocation, bool bIsCritical = false);

	/** Show a hit direction indicator from a world-space source. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void ShowHitDirection(FVector SourceLocation);

	/** Show a toast notification (top-center, fades out). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void ShowToast(const FString& Message, FColor Color = FColor::White);

	/** Flash the screen a color (for parry, finisher, etc). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void FlashScreen(FLinearColor Color = FLinearColor::White, float Duration = 0.15f);

	/** Show a tutorial hint (stays until dismissed or time expires). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void ShowTutorial(const FString& Title, const FString& Body, float Duration = 6.0f);

	/** Dismiss the current tutorial hint. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void DismissTutorial();

	/** Show the game over screen. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void ShowGameOver();

	/** Show the encounter complete screen with grade. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void ShowEncounterComplete();

	/** Show credits scroll. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void ShowCredits();

	/** Show/hide the pause overlay. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void SetPauseVisible(bool bVisible);

	/** Set the active boss for the boss health bar. nullptr to hide. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void SetActiveBoss(class ABossBase* Boss);

protected:
	void DrawResourceBars(AAnansiCharacter* Anansi);
	void DrawComboDisplay(AAnansiCharacter* Anansi);
	void DrawCrosshair();
	void DrawDamageNumbers(float DeltaTime);
	void DrawEnemyHealthBars();
	void DrawBossHealthBar();
	void DrawGameOverScreen();
	void DrawPauseScreen();
	void DrawStateDisplay(AAnansiCharacter* Anansi);
	void DrawCompass(AAnansiCharacter* Anansi);
	void DrawEnemyIndicators(AAnansiCharacter* Anansi);
	void DrawAbilityCooldowns(AAnansiCharacter* Anansi);
	void DrawEncounterComplete();
	void DrawDialogue();
	void DrawStealthIndicator(AAnansiCharacter* Anansi);
	void DrawHitDirectionIndicators(float DeltaTime);
	void DrawInteractionPrompt(AAnansiCharacter* Anansi);
	void DrawMinimap(AAnansiCharacter* Anansi);
	void DrawEnemyStaggerBars();
	void DrawToasts(float DeltaTime);
	void DrawSpeedrunTimer();
	void DrawEnemyAwarenessIcons();
	void DrawComboTimerBar(AAnansiCharacter* Anansi);
	void DrawLockOnReticle(AAnansiCharacter* Anansi);
	void DrawDifficultyDisplay();
	void DrawCreditsScreen(float DeltaTime);
	void DrawQuestObjective();
	void DrawTutorialHint(float DeltaTime);
	void DrawXPBar();

private:
	struct FFloatingDamage
	{
		float Damage;
		FVector WorldLocation;
		float TimeRemaining;
		bool bIsCritical;
		FVector Velocity;
	};

	TArray<FFloatingDamage> ActiveDamageNumbers;

	struct FHitIndicator
	{
		float Angle; // Radians from forward
		float TimeRemaining;
	};
	TArray<FHitIndicator> ActiveHitIndicators;

	struct FToastMessage
	{
		FString Text;
		FColor Color;
		float TimeRemaining;
	};
	TArray<FToastMessage> ActiveToasts;

	FLinearColor ScreenFlashColor = FLinearColor::Transparent;
	float ScreenFlashTimer = 0.0f;
	float ScreenFlashDuration = 0.15f;

	FString TutorialTitle;
	FString TutorialBody;
	float TutorialTimer = 0.0f;

	UPROPERTY()
	TWeakObjectPtr<class ABossBase> ActiveBoss;

	bool bShowGameOver = false;
	bool bShowPause = false;
	bool bShowEncounterComplete = false;
	bool bShowCredits = false;
	float GameOverTimer = 0.0f;
	float EncounterCompleteTimer = 0.0f;
	float CreditsScrollY = 0.0f;

	void DrawBar(float X, float Y, float Width, float Height,
		float Percent, FLinearColor FillColor, FLinearColor BackColor, const FString& Label);
};
