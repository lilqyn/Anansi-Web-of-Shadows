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

	/** Show the game over screen. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void ShowGameOver();

	/** Show the encounter complete screen with grade. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|HUD")
	void ShowEncounterComplete();

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

	UPROPERTY()
	TWeakObjectPtr<class ABossBase> ActiveBoss;

	bool bShowGameOver = false;
	bool bShowPause = false;
	bool bShowEncounterComplete = false;
	float GameOverTimer = 0.0f;
	float EncounterCompleteTimer = 0.0f;

	void DrawBar(float X, float Y, float Width, float Height,
		float Percent, FLinearColor FillColor, FLinearColor BackColor, const FString& Label);
};
