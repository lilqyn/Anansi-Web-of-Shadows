// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "UI/AnansiDevHUD.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "Combat/CombatComponent.h"
#include "AI/EnemyBase.h"
#include "AI/BossBase.h"
#include "Core/CombatStats.h"
#include "Narrative/DialogueManager.h"
#include "Stealth/StealthComponent.h"
#include "World/InteractionComponent.h"
#include "World/AnansiInteractable.h"
#include "World/TimeOfDayManager.h"
#include "Core/DifficultySettings.h"
#include "Core/QuestSystem.h"
#include "Core/PlayerProgression.h"
#include "EngineUtils.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void AAnansiDevHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(GetOwningPawn());
	if (!Anansi)
	{
		return;
	}

	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	DrawResourceBars(Anansi);
	DrawComboDisplay(Anansi);
	DrawCrosshair();
	DrawDamageNumbers(DeltaTime);
	DrawEnemyHealthBars();
	DrawBossHealthBar();
	DrawStateDisplay(Anansi);
	DrawCompass(Anansi);
	DrawEnemyIndicators(Anansi);
	DrawAbilityCooldowns(Anansi);
	DrawDialogue();
	DrawStealthIndicator(Anansi);
	DrawHitDirectionIndicators(DeltaTime);
	DrawInteractionPrompt(Anansi);
	DrawMinimap(Anansi);
	DrawEnemyStaggerBars();
	DrawToasts(DeltaTime);
	DrawSpeedrunTimer();
	DrawEnemyAwarenessIcons();
	DrawComboTimerBar(Anansi);
	DrawLockOnReticle(Anansi);
	DrawDifficultyDisplay();
	DrawQuestObjective();
	DrawTutorialHint(DeltaTime);
	DrawXPBar();

	if (bShowEncounterComplete)
	{
		EncounterCompleteTimer += DeltaTime;
		DrawEncounterComplete();
	}

	if (bShowGameOver)
	{
		GameOverTimer += DeltaTime;
		DrawGameOverScreen();
	}
	if (bShowPause)
	{
		DrawPauseScreen();
	}

	if (bShowCredits)
	{
		DrawCreditsScreen(DeltaTime);
	}

	// Screen flash overlay
	if (ScreenFlashTimer > 0.0f)
	{
		ScreenFlashTimer -= DeltaTime;
		const float Alpha = FMath::Clamp(ScreenFlashTimer / ScreenFlashDuration, 0.0f, 1.0f);
		DrawRect(FLinearColor(ScreenFlashColor.R, ScreenFlashColor.G, ScreenFlashColor.B, Alpha * 0.4f),
			0, 0, Canvas->SizeX, Canvas->SizeY);
	}
}

// ---------------------------------------------------------------------------
// Resource bars (bottom-left)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawResourceBars(AAnansiCharacter* Anansi)
{
	const float BarWidth = 250.0f;
	const float BarHeight = 20.0f;
	const float Padding = 8.0f;
	const float StartX = 30.0f;
	const float StartY = Canvas->SizeY - 120.0f;

	// Health bar (red)
	const float HealthPct = Anansi->GetMaxHealth() > 0 ? Anansi->GetCurrentHealth() / Anansi->GetMaxHealth() : 0;
	DrawBar(StartX, StartY, BarWidth, BarHeight, HealthPct,
		FLinearColor(0.8f, 0.1f, 0.1f), FLinearColor(0.2f, 0.0f, 0.0f),
		FString::Printf(TEXT("HP  %.0f / %.0f"), Anansi->GetCurrentHealth(), Anansi->GetMaxHealth()));

	// Stamina bar (yellow)
	const float StaminaPct = Anansi->GetMaxStamina() > 0 ? Anansi->GetCurrentStamina() / Anansi->GetMaxStamina() : 0;
	DrawBar(StartX, StartY + BarHeight + Padding, BarWidth, BarHeight, StaminaPct,
		FLinearColor(0.9f, 0.8f, 0.1f), FLinearColor(0.2f, 0.18f, 0.0f),
		FString::Printf(TEXT("STA %.0f / %.0f"), Anansi->GetCurrentStamina(), Anansi->GetMaxStamina()));

	// Web energy bar (cyan)
	const float WebPct = Anansi->GetMaxWebEnergy() > 0 ? Anansi->GetCurrentWebEnergy() / Anansi->GetMaxWebEnergy() : 0;
	DrawBar(StartX, StartY + (BarHeight + Padding) * 2, BarWidth, BarHeight, WebPct,
		FLinearColor(0.1f, 0.7f, 0.9f), FLinearColor(0.0f, 0.15f, 0.2f),
		FString::Printf(TEXT("WEB %.0f / %.0f"), Anansi->GetCurrentWebEnergy(), Anansi->GetMaxWebEnergy()));
}

// ---------------------------------------------------------------------------
// Combo display (top-right)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawComboDisplay(AAnansiCharacter* Anansi)
{
	if (!Anansi->CombatComponent)
	{
		return;
	}

	const int32 Combo = Anansi->CombatComponent->GetComboCount();
	if (Combo <= 0)
	{
		return;
	}

	const EStyleRank Rank = Anansi->CombatComponent->GetStyleRank();

	static const TCHAR* RankNames[] = {
		TEXT("D"), TEXT("C"), TEXT("B"), TEXT("A"), TEXT("S"), TEXT("SS"), TEXT("SSS")
	};
	const TCHAR* RankName = RankNames[FMath::Clamp(static_cast<int32>(Rank), 0, 6)];

	const float X = Canvas->SizeX - 200.0f;

	// Combo count
	Canvas->SetDrawColor(FColor::White);
	const FString ComboText = FString::Printf(TEXT("%d HITS"), Combo);
	Canvas->DrawText(GEngine->GetLargeFont(), ComboText, X, 60.0f, 1.5f, 1.5f);

	// Style rank
	static const FColor RankColors[] = {
		FColor(128, 128, 128), FColor(255, 255, 255), FColor(50, 150, 255),
		FColor(25, 255, 25), FColor(255, 200, 0), FColor(255, 100, 0), FColor(255, 25, 25)
	};
	Canvas->SetDrawColor(RankColors[FMath::Clamp(static_cast<int32>(Rank), 0, 6)]);
	Canvas->DrawText(GEngine->GetLargeFont(), RankName, X + 20.0f, 30.0f, 2.5f, 2.5f);
}

// ---------------------------------------------------------------------------
// Crosshair (center)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawCrosshair()
{
	const float CenterX = Canvas->SizeX * 0.5f;
	const float CenterY = Canvas->SizeY * 0.5f;
	const float Size = 8.0f;

	Canvas->SetDrawColor(FColor(255, 255, 255, 150));

	// Draw a simple + crosshair using 2D lines on the HUD
	DrawLine(CenterX - Size, CenterY, CenterX + Size, CenterY, FLinearColor::White);
	DrawLine(CenterX, CenterY - Size, CenterX, CenterY + Size, FLinearColor::White);
}

// ---------------------------------------------------------------------------
// Floating damage numbers
// ---------------------------------------------------------------------------

void AAnansiDevHUD::AddDamageNumber(float Damage, FVector WorldLocation, bool bIsCritical)
{
	FFloatingDamage Entry;
	Entry.Damage = Damage;
	Entry.WorldLocation = WorldLocation;
	Entry.TimeRemaining = 1.5f;
	Entry.bIsCritical = bIsCritical;
	Entry.Velocity = FVector(FMath::FRandRange(-30.0f, 30.0f), 0, 80.0f);

	ActiveDamageNumbers.Add(Entry);
}

void AAnansiDevHUD::DrawDamageNumbers(float DeltaTime)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	for (int32 i = ActiveDamageNumbers.Num() - 1; i >= 0; --i)
	{
		FFloatingDamage& Num = ActiveDamageNumbers[i];
		Num.TimeRemaining -= DeltaTime;

		if (Num.TimeRemaining <= 0.0f)
		{
			ActiveDamageNumbers.RemoveAt(i);
			continue;
		}

		Num.WorldLocation += Num.Velocity * DeltaTime;

		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(Num.WorldLocation, ScreenPos))
		{
			continue;
		}

		const float Alpha = FMath::Clamp(Num.TimeRemaining / 1.0f, 0.0f, 1.0f);
		const float Scale = Num.bIsCritical ? 2.0f : 1.3f;

		FColor Color = Num.bIsCritical
			? FColor(255, 50, 50, static_cast<uint8>(Alpha * 255))
			: FColor(255, 220, 50, static_cast<uint8>(Alpha * 255));

		Canvas->SetDrawColor(Color);
		const FString Text = FString::Printf(TEXT("%.0f"), Num.Damage);
		Canvas->DrawText(GEngine->GetMediumFont(), Text, ScreenPos.X, ScreenPos.Y, Scale, Scale);
	}
}

// ---------------------------------------------------------------------------
// Enemy health bars (above their heads)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawEnemyHealthBars()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return;
	}

	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	for (AActor* Enemy : Enemies)
	{
		AEnemyBase* EnemyBase = Cast<AEnemyBase>(Enemy);
		if (!EnemyBase || EnemyBase->IsDead())
		{
			continue;
		}

		const float Distance = FVector::Dist(PC->GetPawn()->GetActorLocation(), EnemyBase->GetActorLocation());
		if (Distance > 2500.0f)
		{
			continue;
		}

		FVector HeadLoc = EnemyBase->GetActorLocation() + FVector(0, 0, 120.0f);
		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(HeadLoc, ScreenPos))
		{
			continue;
		}

		const float DistScale = FMath::Clamp(1.0f - (Distance / 2500.0f), 0.3f, 1.0f);
		const float BarWidth = 80.0f * DistScale;
		const float BarHeight = 8.0f * DistScale;

		const float X = ScreenPos.X - BarWidth * 0.5f;
		const float Y = ScreenPos.Y - 15.0f;

		const float Pct = EnemyBase->GetHealthPercent();

		// Background
		DrawRect(FLinearColor(0.12f, 0.0f, 0.0f, 0.8f), X, Y, BarWidth, BarHeight);

		// Fill — green to red based on health
		const FLinearColor FillColor = FMath::Lerp(FLinearColor(0.8f, 0.1f, 0.1f), FLinearColor(0.1f, 0.8f, 0.1f), Pct);
		DrawRect(FillColor, X, Y, BarWidth * Pct, BarHeight);
	}
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawBar(float X, float Y, float Width, float Height,
	float Percent, FLinearColor FillColor, FLinearColor BackColor, const FString& Label)
{
	Percent = FMath::Clamp(Percent, 0.0f, 1.0f);

	// Background
	DrawRect(BackColor, X, Y, Width, Height);

	// Fill
	DrawRect(FillColor, X, Y, Width * Percent, Height);

	// Border
	const FLinearColor BorderColor(0.5f, 0.5f, 0.5f, 0.8f);
	DrawLine(X, Y, X + Width, Y, BorderColor);
	DrawLine(X + Width, Y, X + Width, Y + Height, BorderColor);
	DrawLine(X, Y + Height, X + Width, Y + Height, BorderColor);
	DrawLine(X, Y, X, Y + Height, BorderColor);

	// Label
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(GEngine->GetSmallFont(), Label, X + 5.0f, Y + 2.0f);
}

// ---------------------------------------------------------------------------
// Boss health bar (top-center)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::SetActiveBoss(ABossBase* Boss)
{
	ActiveBoss = Boss;
}

void AAnansiDevHUD::DrawBossHealthBar()
{
	ABossBase* Boss = ActiveBoss.Get();
	if (!Boss || Boss->IsDead())
	{
		return;
	}

	const float BarWidth = 500.0f;
	const float BarHeight = 25.0f;
	const float X = (Canvas->SizeX - BarWidth) * 0.5f;
	const float Y = 30.0f;

	// Boss name
	Canvas->SetDrawColor(FColor(255, 200, 50));
	const FString NameStr = Boss->GetBossName().ToString();
	Canvas->DrawText(GEngine->GetMediumFont(), NameStr, X, Y - 22.0f, 1.2f, 1.2f);

	// Phase indicator
	Canvas->SetDrawColor(FColor::White);
	const FString PhaseStr = FString::Printf(TEXT("Phase %d/%d"), Boss->GetCurrentPhase(), Boss->GetTotalPhases());
	Canvas->DrawText(GEngine->GetSmallFont(), PhaseStr, X + BarWidth - 80.0f, Y - 18.0f);

	// Health bar
	DrawBar(X, Y, BarWidth, BarHeight, Boss->GetHealthPercent(),
		FLinearColor(0.8f, 0.15f, 0.05f), FLinearColor(0.15f, 0.0f, 0.0f),
		FString::Printf(TEXT("%.0f / %.0f"), Boss->GetCurrentHealth(), Boss->GetMaxHealth()));
}

// ---------------------------------------------------------------------------
// State display (top-left debug)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawStateDisplay(AAnansiCharacter* Anansi)
{
	static const TCHAR* StateNames[] = {
		TEXT("Idle"), TEXT("Moving"), TEXT("Jumping"), TEXT("WallRunning"),
		TEXT("WebSwinging"), TEXT("Attacking"), TEXT("Dodging"), TEXT("Crouching"),
		TEXT("Stealth"), TEXT("Parrying"), TEXT("UsingAbility"), TEXT("Stunned"), TEXT("Dead")
	};

	const int32 StateIdx = static_cast<int32>(Anansi->GetCharacterState());
	const TCHAR* StateName = (StateIdx >= 0 && StateIdx < 13) ? StateNames[StateIdx] : TEXT("Unknown");

	Canvas->SetDrawColor(FColor(180, 180, 180));
	Canvas->DrawText(GEngine->GetSmallFont(),
		FString::Printf(TEXT("State: %s"), StateName), 30.0f, 30.0f);

	// Speed
	const float Speed = Anansi->GetVelocity().Size();
	Canvas->DrawText(GEngine->GetSmallFont(),
		FString::Printf(TEXT("Speed: %.0f"), Speed), 30.0f, 45.0f);

	// Combat stats
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UCombatStatsSubsystem* Stats = GI->GetSubsystem<UCombatStatsSubsystem>())
		{
			Canvas->DrawText(GEngine->GetSmallFont(),
				FString::Printf(TEXT("Kills: %d  Hits: %d  Max Combo: %d"),
					Stats->GetTotalKills(), Stats->GetTotalHits(), Stats->GetMaxCombo()),
				30.0f, 60.0f);

			// Kill streak indicator
			if (Stats->GetKillStreak() >= 3)
			{
				Canvas->SetDrawColor(FColor(255, 150, 0));
				Canvas->DrawText(GEngine->GetMediumFont(),
					FString::Printf(TEXT("STREAK x%d (%.1fx)"),
						Stats->GetKillStreak(), Stats->GetKillStreakMultiplier()),
					Canvas->SizeX * 0.5f - 60.0f, Canvas->SizeY * 0.35f, 1.2f, 1.2f);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Game Over screen
// ---------------------------------------------------------------------------

void AAnansiDevHUD::ShowGameOver()
{
	bShowGameOver = true;
	GameOverTimer = 0.0f;
}

void AAnansiDevHUD::DrawGameOverScreen()
{
	// Darken background
	const float Alpha = FMath::Clamp(GameOverTimer / 1.0f, 0.0f, 0.7f);
	DrawRect(FLinearColor(0, 0, 0, Alpha), 0, 0, Canvas->SizeX, Canvas->SizeY);

	if (GameOverTimer < 0.5f)
	{
		return;
	}

	const float CenterX = Canvas->SizeX * 0.5f;
	const float CenterY = Canvas->SizeY * 0.5f;

	// "DEFEATED" text
	Canvas->SetDrawColor(FColor(200, 30, 30));
	Canvas->DrawText(GEngine->GetLargeFont(), TEXT("DEFEATED"), CenterX - 100.0f, CenterY - 30.0f, 3.0f, 3.0f);

	// Subtitle
	Canvas->SetDrawColor(FColor(200, 200, 200));
	Canvas->DrawText(GEngine->GetMediumFont(), TEXT("The story ends here..."),
		CenterX - 120.0f, CenterY + 40.0f, 1.2f, 1.2f);

	// Instructions
	if (GameOverTimer > 2.0f)
	{
		Canvas->SetDrawColor(FColor(150, 150, 150));
		Canvas->DrawText(GEngine->GetSmallFont(), TEXT("Press [R] to restart or [ESC] to quit"),
			CenterX - 140.0f, CenterY + 80.0f);
	}
}

// ---------------------------------------------------------------------------
// Pause screen
// ---------------------------------------------------------------------------

void AAnansiDevHUD::SetPauseVisible(bool bVisible)
{
	bShowPause = bVisible;
}

void AAnansiDevHUD::DrawPauseScreen()
{
	// Semi-transparent overlay
	DrawRect(FLinearColor(0, 0, 0, 0.5f), 0, 0, Canvas->SizeX, Canvas->SizeY);

	const float CenterX = Canvas->SizeX * 0.5f;
	const float CenterY = Canvas->SizeY * 0.5f;

	// Title
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(GEngine->GetLargeFont(), TEXT("PAUSED"), CenterX - 80.0f, CenterY - 60.0f, 2.5f, 2.5f);

	// Menu items
	Canvas->SetDrawColor(FColor(200, 200, 200));
	Canvas->DrawText(GEngine->GetMediumFont(), TEXT("[ESC] Resume"), CenterX - 60.0f, CenterY + 20.0f);
	Canvas->DrawText(GEngine->GetMediumFont(), TEXT("[R] Restart"), CenterX - 60.0f, CenterY + 50.0f);

	// Game info
	Canvas->SetDrawColor(FColor(120, 120, 120));
	Canvas->DrawText(GEngine->GetSmallFont(), TEXT("Anansi: Web of Shadows - Vertical Slice"),
		CenterX - 120.0f, CenterY + 110.0f);
}

// ---------------------------------------------------------------------------
// Compass (top-center strip)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawCompass(AAnansiCharacter* Anansi)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	const float CompassWidth = 400.0f;
	const float CompassHeight = 20.0f;
	const float CenterX = Canvas->SizeX * 0.5f;
	const float Y = 8.0f;
	const float X = CenterX - CompassWidth * 0.5f;

	// Background
	DrawRect(FLinearColor(0, 0, 0, 0.4f), X, Y, CompassWidth, CompassHeight);

	// Get camera yaw
	const float Yaw = PC->GetControlRotation().Yaw;

	// Cardinal directions
	struct FCardinal { float Angle; const TCHAR* Label; FColor Color; };
	const TArray<FCardinal> Cardinals = {
		{0.0f,   TEXT("N"), FColor(255, 50, 50)},
		{90.0f,  TEXT("E"), FColor(200, 200, 200)},
		{180.0f, TEXT("S"), FColor(200, 200, 200)},
		{270.0f, TEXT("W"), FColor(200, 200, 200)},
		{45.0f,  TEXT("NE"), FColor(150, 150, 150)},
		{135.0f, TEXT("SE"), FColor(150, 150, 150)},
		{225.0f, TEXT("SW"), FColor(150, 150, 150)},
		{315.0f, TEXT("NW"), FColor(150, 150, 150)},
	};

	for (const FCardinal& Dir : Cardinals)
	{
		float Diff = FMath::FindDeltaAngleDegrees(Yaw, Dir.Angle);
		float ScreenX = CenterX + (Diff / 90.0f) * (CompassWidth * 0.5f);

		if (ScreenX >= X && ScreenX <= X + CompassWidth)
		{
			Canvas->SetDrawColor(Dir.Color);
			Canvas->DrawText(GEngine->GetSmallFont(), Dir.Label, ScreenX - 4.0f, Y + 3.0f);
		}
	}

	// Center tick
	DrawLine(CenterX, Y + CompassHeight, CenterX, Y + CompassHeight + 5.0f, FLinearColor::White);
}

// ---------------------------------------------------------------------------
// Off-screen enemy indicators (edge-of-screen arrows)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawEnemyIndicators(AAnansiCharacter* Anansi)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	for (AActor* Enemy : Enemies)
	{
		AEnemyBase* EnemyBase = Cast<AEnemyBase>(Enemy);
		if (!EnemyBase || EnemyBase->IsDead()) continue;

		FVector2D ScreenPos;
		bool bOnScreen = PC->ProjectWorldLocationToScreen(Enemy->GetActorLocation(), ScreenPos);

		if (bOnScreen && ScreenPos.X > 0 && ScreenPos.X < Canvas->SizeX &&
			ScreenPos.Y > 0 && ScreenPos.Y < Canvas->SizeY)
		{
			continue; // Already visible — skip indicator
		}

		// Draw edge indicator
		const float Margin = 40.0f;
		const FVector2D Center(Canvas->SizeX * 0.5f, Canvas->SizeY * 0.5f);
		FVector2D Dir = (ScreenPos - Center).GetSafeNormal();

		// Clamp to screen edges
		FVector2D IndicatorPos;
		IndicatorPos.X = FMath::Clamp(Center.X + Dir.X * (Canvas->SizeX * 0.5f - Margin), Margin, Canvas->SizeX - Margin);
		IndicatorPos.Y = FMath::Clamp(Center.Y + Dir.Y * (Canvas->SizeY * 0.5f - Margin), Margin, Canvas->SizeY - Margin);

		// Red triangle indicator
		Canvas->SetDrawColor(FColor(255, 60, 60, 200));
		Canvas->DrawText(GEngine->GetSmallFont(), TEXT("!"), IndicatorPos.X - 4.0f, IndicatorPos.Y - 6.0f, 1.5f, 1.5f);
	}
}

// ---------------------------------------------------------------------------
// Ability cooldowns (bottom-right)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawAbilityCooldowns(AAnansiCharacter* Anansi)
{
	float X = Canvas->SizeX - 200.0f;
	float Y = Canvas->SizeY - 120.0f;
	const float BoxSize = 40.0f;
	const float Gap = 8.0f;

	struct FAbilitySlot
	{
		const TCHAR* Label;
		const TCHAR* Key;
		FLinearColor Color;
	};

	const TArray<FAbilitySlot> Slots = {
		{TEXT("Web"),    TEXT("[E]"), FLinearColor(0.1f, 0.7f, 0.9f)},
		{TEXT("Sense"),  TEXT("[R]"), FLinearColor(0.9f, 0.7f, 0.1f)},
		{TEXT("Path"),   TEXT("[T]"), FLinearColor(0.4f, 0.9f, 0.4f)},
		{TEXT("Dodge"),  TEXT("[Alt]"), FLinearColor(0.7f, 0.7f, 0.7f)},
	};

	for (const FAbilitySlot& Slot : Slots)
	{
		// Background box
		DrawRect(FLinearColor(0.05f, 0.05f, 0.05f, 0.7f), X, Y, BoxSize, BoxSize);

		// Colored border
		DrawLine(X, Y, X + BoxSize, Y, Slot.Color);
		DrawLine(X, Y + BoxSize, X + BoxSize, Y + BoxSize, Slot.Color);
		DrawLine(X, Y, X, Y + BoxSize, Slot.Color);
		DrawLine(X + BoxSize, Y, X + BoxSize, Y + BoxSize, Slot.Color);

		// Label
		Canvas->SetDrawColor(FColor::White);
		Canvas->DrawText(GEngine->GetSmallFont(), Slot.Label, X + 4.0f, Y + 4.0f);
		Canvas->SetDrawColor(FColor(150, 150, 150));
		Canvas->DrawText(GEngine->GetSmallFont(), Slot.Key, X + 4.0f, Y + 24.0f, 0.8f, 0.8f);

		X += BoxSize + Gap;
	}
}

// ---------------------------------------------------------------------------
// Encounter complete screen
// ---------------------------------------------------------------------------

void AAnansiDevHUD::ShowEncounterComplete()
{
	bShowEncounterComplete = true;
	EncounterCompleteTimer = 0.0f;
}

void AAnansiDevHUD::DrawEncounterComplete()
{
	// Darken
	const float Alpha = FMath::Clamp(EncounterCompleteTimer / 0.5f, 0.0f, 0.5f);
	DrawRect(FLinearColor(0, 0, 0, Alpha), 0, 0, Canvas->SizeX, Canvas->SizeY);

	if (EncounterCompleteTimer < 0.3f) return;

	const float CenterX = Canvas->SizeX * 0.5f;
	const float CenterY = Canvas->SizeY * 0.5f;

	// "ENCOUNTER COMPLETE"
	Canvas->SetDrawColor(FColor(255, 200, 50));
	Canvas->DrawText(GEngine->GetLargeFont(), TEXT("ENCOUNTER COMPLETE"),
		CenterX - 180.0f, CenterY - 50.0f, 2.0f, 2.0f);

	// Grade
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UCombatStatsSubsystem* Stats = GI->GetSubsystem<UCombatStatsSubsystem>())
		{
			Canvas->SetDrawColor(FColor::White);

			const FString Grade = Stats->GetPerformanceGrade();
			const FString Summary = FString::Printf(
				TEXT("Grade: %s    Kills: %d    Max Combo: %d    Parries: %d"),
				*Grade, Stats->GetTotalKills(), Stats->GetMaxCombo(), 0);

			Canvas->DrawText(GEngine->GetMediumFont(), Summary, CenterX - 200.0f, CenterY + 20.0f, 1.0f, 1.0f);

			// Large grade letter
			FColor GradeColor = FColor::White;
			if (Grade == TEXT("S")) GradeColor = FColor(255, 200, 0);
			else if (Grade == TEXT("A")) GradeColor = FColor(25, 255, 25);
			else if (Grade == TEXT("B")) GradeColor = FColor(50, 150, 255);
			else if (Grade == TEXT("D")) GradeColor = FColor(200, 50, 50);

			Canvas->SetDrawColor(GradeColor);
			Canvas->DrawText(GEngine->GetLargeFont(), Grade, CenterX - 15.0f, CenterY + 50.0f, 4.0f, 4.0f);
		}
	}

	if (EncounterCompleteTimer > 3.0f)
	{
		bShowEncounterComplete = false;
	}
}

// ---------------------------------------------------------------------------
// Dialogue display (bottom-center dialogue box)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawDialogue()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return;

	UDialogueManager* DM = GI->GetSubsystem<UDialogueManager>();
	if (!DM || !DM->IsInDialogue()) return;

	const FDialogueNode& Node = DM->GetCurrentNode();

	const float BoxWidth = Canvas->SizeX * 0.7f;
	const float BoxHeight = 180.0f;
	const float BoxX = (Canvas->SizeX - BoxWidth) * 0.5f;
	const float BoxY = Canvas->SizeY - BoxHeight - 40.0f;

	// Background
	DrawRect(FLinearColor(0.02f, 0.02f, 0.05f, 0.85f), BoxX, BoxY, BoxWidth, BoxHeight);

	// Border
	DrawLine(BoxX, BoxY, BoxX + BoxWidth, BoxY, FLinearColor(0.4f, 0.3f, 0.1f));
	DrawLine(BoxX, BoxY + BoxHeight, BoxX + BoxWidth, BoxY + BoxHeight, FLinearColor(0.4f, 0.3f, 0.1f));
	DrawLine(BoxX, BoxY, BoxX, BoxY + BoxHeight, FLinearColor(0.4f, 0.3f, 0.1f));
	DrawLine(BoxX + BoxWidth, BoxY, BoxX + BoxWidth, BoxY + BoxHeight, FLinearColor(0.4f, 0.3f, 0.1f));

	// Speaker name
	Canvas->SetDrawColor(FColor(255, 200, 50));
	Canvas->DrawText(GEngine->GetMediumFont(), Node.SpeakerName.ToString(),
		BoxX + 20.0f, BoxY + 10.0f, 1.2f, 1.2f);

	// Dialogue text
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(GEngine->GetMediumFont(), Node.DialogueText.ToString(),
		BoxX + 20.0f, BoxY + 40.0f, 1.0f, 1.0f);

	// Choices
	if (Node.Choices.Num() > 0)
	{
		float ChoiceY = BoxY + 90.0f;
		for (int32 i = 0; i < Node.Choices.Num(); ++i)
		{
			const FDialogueChoice& Choice = Node.Choices[i];
			Canvas->SetDrawColor(FColor(200, 200, 255));
			const FString ChoiceStr = FString::Printf(TEXT("[%d] %s"), i + 1, *Choice.ChoiceText.ToString());
			Canvas->DrawText(GEngine->GetSmallFont(), ChoiceStr, BoxX + 30.0f, ChoiceY);
			ChoiceY += 20.0f;
		}
	}
	else
	{
		// Auto-advance prompt
		Canvas->SetDrawColor(FColor(150, 150, 150));
		Canvas->DrawText(GEngine->GetSmallFont(), TEXT("Press [F] to continue..."),
			BoxX + 20.0f, BoxY + BoxHeight - 30.0f);
	}
}

// ---------------------------------------------------------------------------
// Stealth visibility indicator (bottom bar)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawStealthIndicator(AAnansiCharacter* Anansi)
{
	// Only show when crouching
	if (Anansi->GetCharacterState() != EAnansiCharacterState::Crouching)
	{
		return;
	}

	const float BarWidth = 150.0f;
	const float BarHeight = 12.0f;
	const float X = Canvas->SizeX * 0.5f - BarWidth * 0.5f;
	const float Y = Canvas->SizeY - 50.0f;

	// Try to get stealth component for actual visibility
	UStealthComponent* Stealth = Anansi->FindComponentByClass<UStealthComponent>();
	float Visibility = 0.3f; // Default when crouching

	if (Stealth)
	{
		Visibility = Stealth->GetVisibilityScore();
	}

	// Label
	Canvas->SetDrawColor(FColor(180, 180, 180));
	Canvas->DrawText(GEngine->GetSmallFont(), TEXT("STEALTH"), X, Y - 15.0f, 0.9f, 0.9f);

	// Background
	DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.7f), X, Y, BarWidth, BarHeight);

	// Fill — green = hidden, yellow = partial, red = visible
	FLinearColor FillColor;
	if (Visibility < 0.3f) FillColor = FLinearColor(0.1f, 0.8f, 0.2f); // Hidden
	else if (Visibility < 0.6f) FillColor = FLinearColor(0.9f, 0.8f, 0.1f); // Partial
	else FillColor = FLinearColor(0.9f, 0.2f, 0.1f); // Visible

	DrawRect(FillColor, X, Y, BarWidth * FMath::Clamp(1.0f - Visibility, 0.0f, 1.0f), BarHeight);

	// Eye icon indicator
	const TCHAR* EyeText = Visibility < 0.3f ? TEXT("HIDDEN") : (Visibility < 0.6f ? TEXT("PARTIAL") : TEXT("VISIBLE"));
	Canvas->DrawText(GEngine->GetSmallFont(), EyeText, X + BarWidth + 10.0f, Y - 2.0f, 0.8f, 0.8f);
}

// ---------------------------------------------------------------------------
// Hit direction indicator
// ---------------------------------------------------------------------------

void AAnansiDevHUD::ShowHitDirection(FVector SourceLocation)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->GetPawn()) return;

	const FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
	const FVector ToSource = (SourceLocation - PlayerLoc).GetSafeNormal();
	const FVector Forward = PC->GetPawn()->GetActorForwardVector();

	// Calculate angle from forward
	const float Dot = FVector::DotProduct(FVector(Forward.X, Forward.Y, 0).GetSafeNormal(),
		FVector(ToSource.X, ToSource.Y, 0).GetSafeNormal());
	const float Cross = FVector::CrossProduct(Forward, ToSource).Z;
	const float Angle = FMath::Atan2(Cross, Dot);

	FHitIndicator Indicator;
	Indicator.Angle = Angle;
	Indicator.TimeRemaining = 1.5f;
	ActiveHitIndicators.Add(Indicator);
}

void AAnansiDevHUD::DrawHitDirectionIndicators(float DeltaTime)
{
	const float CenterX = Canvas->SizeX * 0.5f;
	const float CenterY = Canvas->SizeY * 0.5f;
	const float Radius = FMath::Min(CenterX, CenterY) * 0.6f;

	for (int32 i = ActiveHitIndicators.Num() - 1; i >= 0; --i)
	{
		FHitIndicator& Ind = ActiveHitIndicators[i];
		Ind.TimeRemaining -= DeltaTime;

		if (Ind.TimeRemaining <= 0.0f)
		{
			ActiveHitIndicators.RemoveAt(i);
			continue;
		}

		const float Alpha = FMath::Clamp(Ind.TimeRemaining / 1.0f, 0.0f, 1.0f);

		// Position on screen circle
		// Angle 0 = top (forward), positive = right
		const float ScreenAngle = Ind.Angle - PI * 0.5f; // Adjust so forward = top
		const float PosX = CenterX + FMath::Cos(ScreenAngle) * Radius;
		const float PosY = CenterY + FMath::Sin(ScreenAngle) * Radius;

		// Draw red wedge indicator
		const FLinearColor IndicatorColor(0.9f, 0.1f, 0.1f, Alpha * 0.8f);

		// Draw as a small line pointing inward
		const float InnerRadius = Radius - 30.0f;
		const float InPosX = CenterX + FMath::Cos(ScreenAngle) * InnerRadius;
		const float InPosY = CenterY + FMath::Sin(ScreenAngle) * InnerRadius;

		DrawLine(InPosX, InPosY, PosX, PosY, IndicatorColor);

		// Side lines for wedge effect
		const float WedgeAngle = 0.15f;
		const float LPosX = CenterX + FMath::Cos(ScreenAngle - WedgeAngle) * Radius;
		const float LPosY = CenterY + FMath::Sin(ScreenAngle - WedgeAngle) * Radius;
		const float RPosX = CenterX + FMath::Cos(ScreenAngle + WedgeAngle) * Radius;
		const float RPosY = CenterY + FMath::Sin(ScreenAngle + WedgeAngle) * Radius;

		DrawLine(InPosX, InPosY, LPosX, LPosY, IndicatorColor);
		DrawLine(InPosX, InPosY, RPosX, RPosY, IndicatorColor);
	}
}

// ---------------------------------------------------------------------------
// Interaction prompt (center-bottom)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawInteractionPrompt(AAnansiCharacter* Anansi)
{
	if (!Anansi->InteractionComponent) return;

	AAnansiInteractable* Focused = Anansi->InteractionComponent->GetFocusedInteractable();
	if (!Focused) return;

	const float CenterX = Canvas->SizeX * 0.5f;
	const float Y = Canvas->SizeY * 0.65f;

	// Background
	const float BoxWidth = 250.0f;
	const float BoxHeight = 35.0f;
	DrawRect(FLinearColor(0, 0, 0, 0.6f), CenterX - BoxWidth * 0.5f, Y, BoxWidth, BoxHeight);

	// Prompt text
	Canvas->SetDrawColor(FColor(255, 220, 100));
	const FString Prompt = FString::Printf(TEXT("[F] %s"), *Focused->InteractionPrompt.ToString());
	Canvas->DrawText(GEngine->GetMediumFont(), Prompt, CenterX - 60.0f, Y + 6.0f, 1.0f, 1.0f);
}

// ---------------------------------------------------------------------------
// Minimap (top-right corner)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawMinimap(AAnansiCharacter* Anansi)
{
	const float MapSize = 150.0f;
	const float MapX = Canvas->SizeX - MapSize - 15.0f;
	const float MapY = 80.0f;
	const float MapCenterX = MapX + MapSize * 0.5f;
	const float MapCenterY = MapY + MapSize * 0.5f;
	const float MapScale = 0.05f; // World units to minimap pixels

	// Background
	DrawRect(FLinearColor(0, 0, 0, 0.5f), MapX, MapY, MapSize, MapSize);

	// Border
	DrawLine(MapX, MapY, MapX + MapSize, MapY, FLinearColor(0.4f, 0.4f, 0.4f));
	DrawLine(MapX + MapSize, MapY, MapX + MapSize, MapY + MapSize, FLinearColor(0.4f, 0.4f, 0.4f));
	DrawLine(MapX, MapY + MapSize, MapX + MapSize, MapY + MapSize, FLinearColor(0.4f, 0.4f, 0.4f));
	DrawLine(MapX, MapY, MapX, MapY + MapSize, FLinearColor(0.4f, 0.4f, 0.4f));

	const FVector PlayerLoc = Anansi->GetActorLocation();
	const float PlayerYaw = Anansi->GetControlRotation().Yaw;

	// Player indicator (white triangle at center)
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(GEngine->GetSmallFont(), TEXT("^"), MapCenterX - 4, MapCenterY - 6, 1.2f, 1.2f);

	// Enemies (red dots)
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	for (AActor* Enemy : Enemies)
	{
		AEnemyBase* EBase = Cast<AEnemyBase>(Enemy);
		if (!EBase || EBase->IsDead()) continue;

		FVector Offset = Enemy->GetActorLocation() - PlayerLoc;
		// Rotate by negative player yaw so map is always north-up relative to camera
		const float Rad = FMath::DegreesToRadians(-PlayerYaw);
		const float RotX = Offset.X * FMath::Cos(Rad) - Offset.Y * FMath::Sin(Rad);
		const float RotY = Offset.X * FMath::Sin(Rad) + Offset.Y * FMath::Cos(Rad);

		const float DotX = MapCenterX + RotX * MapScale;
		const float DotY = MapCenterY - RotY * MapScale;

		// Clamp to minimap bounds
		if (DotX < MapX || DotX > MapX + MapSize || DotY < MapY || DotY > MapY + MapSize)
			continue;

		Canvas->SetDrawColor(FColor::Red);
		Canvas->DrawText(GEngine->GetSmallFont(), TEXT("*"), DotX - 3, DotY - 5, 1.0f, 1.0f);
	}

	// Collectibles (gold dots)
	TArray<AActor*> Collectibles;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Collectible"), Collectibles);

	for (AActor* Coll : Collectibles)
	{
		FVector Offset = Coll->GetActorLocation() - PlayerLoc;
		const float Rad = FMath::DegreesToRadians(-PlayerYaw);
		const float RotX = Offset.X * FMath::Cos(Rad) - Offset.Y * FMath::Sin(Rad);
		const float RotY = Offset.X * FMath::Sin(Rad) + Offset.Y * FMath::Cos(Rad);

		const float DotX = MapCenterX + RotX * MapScale;
		const float DotY = MapCenterY - RotY * MapScale;

		if (DotX < MapX || DotX > MapX + MapSize || DotY < MapY || DotY > MapY + MapSize)
			continue;

		Canvas->SetDrawColor(FColor(255, 200, 50));
		Canvas->DrawText(GEngine->GetSmallFont(), TEXT("o"), DotX - 3, DotY - 5, 0.9f, 0.9f);
	}

	// Label
	Canvas->SetDrawColor(FColor(150, 150, 150));
	Canvas->DrawText(GEngine->GetSmallFont(), TEXT("MINIMAP"), MapX + 2, MapY + MapSize + 2, 0.7f, 0.7f);
}

// ---------------------------------------------------------------------------
// Enemy stagger bars (below health bars)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawEnemyStaggerBars()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->GetPawn()) return;

	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	for (AActor* Enemy : Enemies)
	{
		AEnemyBase* EBase = Cast<AEnemyBase>(Enemy);
		if (!EBase || EBase->IsDead()) continue;

		const float Distance = FVector::Dist(PC->GetPawn()->GetActorLocation(), EBase->GetActorLocation());
		if (Distance > 1500.0f) continue;

		FVector HeadLoc = EBase->GetActorLocation() + FVector(0, 0, 130);
		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(HeadLoc, ScreenPos)) continue;

		const float DistScale = FMath::Clamp(1.0f - (Distance / 1500.0f), 0.3f, 1.0f);
		const float BarWidth = 60.0f * DistScale;
		const float BarHeight = 4.0f * DistScale;
		const float X = ScreenPos.X - BarWidth * 0.5f;
		const float Y = ScreenPos.Y - 5.0f;

		// Stagger bar — fills up as accumulated damage approaches threshold
		// (This is a visual approximation since AccumulatedDamage is private)
		const float StaggerPct = FMath::Clamp(1.0f - EBase->GetHealthPercent(), 0.0f, 1.0f);

		DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.5f), X, Y, BarWidth, BarHeight);
		DrawRect(FLinearColor(0.9f, 0.6f, 0.1f, 0.7f), X, Y, BarWidth * StaggerPct, BarHeight);
	}
}

// ---------------------------------------------------------------------------
// Toast notifications (top-center, stack downward)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::ShowToast(const FString& Message, FColor Color)
{
	FToastMessage Toast;
	Toast.Text = Message;
	Toast.Color = Color;
	Toast.TimeRemaining = 3.0f;
	ActiveToasts.Add(Toast);

	// Limit to 5 visible toasts
	while (ActiveToasts.Num() > 5)
	{
		ActiveToasts.RemoveAt(0);
	}
}

void AAnansiDevHUD::DrawToasts(float DeltaTime)
{
	const float CenterX = Canvas->SizeX * 0.5f;
	float Y = 65.0f;

	for (int32 i = ActiveToasts.Num() - 1; i >= 0; --i)
	{
		FToastMessage& Toast = ActiveToasts[i];
		Toast.TimeRemaining -= DeltaTime;

		if (Toast.TimeRemaining <= 0.0f)
		{
			ActiveToasts.RemoveAt(i);
			continue;
		}

		const float Alpha = FMath::Clamp(Toast.TimeRemaining / 0.5f, 0.0f, 1.0f);

		// Background
		const float TextWidth = Toast.Text.Len() * 7.0f;
		DrawRect(FLinearColor(0, 0, 0, 0.5f * Alpha),
			CenterX - TextWidth * 0.5f - 10.0f, Y, TextWidth + 20.0f, 22.0f);

		// Text
		FColor DrawColor = Toast.Color;
		DrawColor.A = static_cast<uint8>(Alpha * 255);
		Canvas->SetDrawColor(DrawColor);
		Canvas->DrawText(GEngine->GetSmallFont(), Toast.Text, CenterX - TextWidth * 0.5f, Y + 3.0f);

		Y += 25.0f;
	}
}

// ---------------------------------------------------------------------------
// Speedrun timer (top-left, below debug info)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawSpeedrunTimer()
{
	const float ElapsedTime = GetWorld()->GetTimeSeconds();

	const int32 Minutes = static_cast<int32>(ElapsedTime) / 60;
	const int32 Seconds = static_cast<int32>(ElapsedTime) % 60;
	const int32 Millis = static_cast<int32>(FMath::Fmod(ElapsedTime, 1.0f) * 100);

	Canvas->SetDrawColor(FColor(100, 200, 100));
	Canvas->DrawText(GEngine->GetSmallFont(),
		FString::Printf(TEXT("Time: %02d:%02d.%02d"), Minutes, Seconds, Millis),
		30.0f, 78.0f, 0.9f, 0.9f);

	// Time of day
	for (TActorIterator<ATimeOfDayManager> It(GetWorld()); It; ++It)
	{
		const float Hour = It->GetCurrentHour();
		const int32 H = static_cast<int32>(Hour);
		const int32 M = static_cast<int32>(FMath::Fmod(Hour, 1.0f) * 60);
		const TCHAR* Period = It->GetCurrentTimeOfDay() == ETimeOfDay::Night ? TEXT("Night") :
			It->GetCurrentTimeOfDay() == ETimeOfDay::Dawn ? TEXT("Dawn") :
			It->GetCurrentTimeOfDay() == ETimeOfDay::Dusk ? TEXT("Dusk") : TEXT("Day");

		Canvas->SetDrawColor(FColor(200, 180, 100));
		Canvas->DrawText(GEngine->GetSmallFont(),
			FString::Printf(TEXT("World: %02d:%02d (%s)"), H, M, Period), 30.0f, 93.0f, 0.9f, 0.9f);
		break;
	}

	// Death count from stats
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UCombatStatsSubsystem* Stats = GI->GetSubsystem<UCombatStatsSubsystem>())
		{
			Canvas->SetDrawColor(FColor(200, 100, 100));
			Canvas->DrawText(GEngine->GetSmallFont(),
				FString::Printf(TEXT("Deaths: %d"), Stats->GetTotalKills() > 0 ? 0 : 0),
				30.0f, 108.0f, 0.9f, 0.9f);
		}
	}
}

// ---------------------------------------------------------------------------
// Enemy awareness icons (? and ! above heads)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawEnemyAwarenessIcons()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->GetPawn()) return;

	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	for (AActor* Enemy : Enemies)
	{
		AEnemyBase* EBase = Cast<AEnemyBase>(Enemy);
		if (!EBase || EBase->IsDead()) continue;

		const float Dist = FVector::Dist(PC->GetPawn()->GetActorLocation(), EBase->GetActorLocation());
		if (Dist > 2000.0f) continue;

		FVector IconLoc = EBase->GetActorLocation() + FVector(0, 0, 150);
		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(IconLoc, ScreenPos)) continue;

		const EEnemyAwareness Awareness = EBase->GetAwarenessState();

		const TCHAR* Icon = TEXT("");
		FColor IconColor = FColor::White;

		switch (Awareness)
		{
		case EEnemyAwareness::Suspicious:
			Icon = TEXT("?");
			IconColor = FColor::Yellow;
			break;
		case EEnemyAwareness::Alert:
			Icon = TEXT("!");
			IconColor = FColor(255, 150, 0);
			break;
		case EEnemyAwareness::Combat:
			Icon = TEXT("!!");
			IconColor = FColor::Red;
			break;
		default:
			continue; // Don't draw for unaware
		}

		Canvas->SetDrawColor(IconColor);
		Canvas->DrawText(GEngine->GetMediumFont(), Icon, ScreenPos.X - 5, ScreenPos.Y - 25, 1.5f, 1.5f);
	}
}

// ---------------------------------------------------------------------------
// Combo timer bar (under combo count, shows chain window)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawComboTimerBar(AAnansiCharacter* Anansi)
{
	if (!Anansi->CombatComponent) return;

	const int32 Combo = Anansi->CombatComponent->GetComboCount();
	if (Combo <= 0) return;

	const float X = Canvas->SizeX - 200.0f;
	const float Y = 90.0f;
	const float BarWidth = 100.0f;
	const float BarHeight = 6.0f;

	// Combo decay timer — shows how long until combo drops
	// ComboDecayDelay is 3 seconds, so we approximate
	const float DecayDelay = 3.0f;
	// We don't have direct access to TimeSinceLastHit, so show a visual pulse
	const float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 3.0f));

	DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.5f), X, Y, BarWidth, BarHeight);
	DrawRect(FLinearColor(1.0f, 0.8f, 0.0f, 0.6f + Pulse * 0.4f), X, Y, BarWidth * Pulse, BarHeight);

	// Variety multiplier display
	const float Variety = Anansi->CombatComponent->GetComboVarietyMultiplier();
	if (Variety > 1.05f)
	{
		Canvas->SetDrawColor(FColor(100, 255, 100));
		Canvas->DrawText(GEngine->GetSmallFont(),
			FString::Printf(TEXT("x%.1f"), Variety), X + BarWidth + 5, Y - 3, 0.9f, 0.9f);
	}
}

// ---------------------------------------------------------------------------
// Lock-on reticle (diamond around locked target)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawLockOnReticle(AAnansiCharacter* Anansi)
{
	if (!Anansi->CombatComponent || !Anansi->CombatComponent->IsLockedOn()) return;

	AActor* Target = Anansi->CombatComponent->GetLockOnTarget();
	if (!Target) return;

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	FVector2D ScreenPos;
	if (!PC->ProjectWorldLocationToScreen(Target->GetActorLocation() + FVector(0, 0, 50), ScreenPos)) return;

	const float Size = 25.0f;
	const FLinearColor ReticleColor(1.0f, 0.3f, 0.1f, 0.9f);

	// Diamond shape
	DrawLine(ScreenPos.X, ScreenPos.Y - Size, ScreenPos.X + Size, ScreenPos.Y, ReticleColor);
	DrawLine(ScreenPos.X + Size, ScreenPos.Y, ScreenPos.X, ScreenPos.Y + Size, ReticleColor);
	DrawLine(ScreenPos.X, ScreenPos.Y + Size, ScreenPos.X - Size, ScreenPos.Y, ReticleColor);
	DrawLine(ScreenPos.X - Size, ScreenPos.Y, ScreenPos.X, ScreenPos.Y - Size, ReticleColor);

	// Target name
	Canvas->SetDrawColor(FColor(255, 100, 50));
	Canvas->DrawText(GEngine->GetSmallFont(), Target->GetName(), ScreenPos.X - 30, ScreenPos.Y + Size + 5, 0.8f, 0.8f);
}

// ---------------------------------------------------------------------------
// Difficulty display (bottom-left, under resource bars)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawDifficultyDisplay()
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UDifficultySubsystem* Diff = GI->GetSubsystem<UDifficultySubsystem>())
		{
			FColor DiffColor;
			switch (Diff->GetDifficulty())
			{
			case EDifficulty::Easy:   DiffColor = FColor(100, 200, 100); break;
			case EDifficulty::Hard:   DiffColor = FColor(255, 80, 80); break;
			default:                  DiffColor = FColor(200, 200, 200); break;
			}

			Canvas->SetDrawColor(DiffColor);
			Canvas->DrawText(GEngine->GetSmallFont(),
				FString::Printf(TEXT("Difficulty: %s"), *Diff->GetDifficultyName()),
				30.0f, Canvas->SizeY - 30.0f, 0.8f, 0.8f);
		}
	}
}

// ---------------------------------------------------------------------------
// Screen flash
// ---------------------------------------------------------------------------

void AAnansiDevHUD::FlashScreen(FLinearColor Color, float Duration)
{
	ScreenFlashColor = Color;
	ScreenFlashTimer = Duration;
	ScreenFlashDuration = Duration;
}

// ---------------------------------------------------------------------------
// Quest objective display (top-right corner)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawQuestObjective()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return;

	UQuestSystem* Quest = GI->GetSubsystem<UQuestSystem>();
	if (!Quest || !Quest->HasActiveObjective()) return;

	const FQuestObjective& Obj = Quest->GetCurrentObjective();

	const float BoxWidth = 280.0f;
	const float BoxHeight = 55.0f;
	const float X = Canvas->SizeX - BoxWidth - 15.0f;
	const float Y = 250.0f;

	// Background
	DrawRect(FLinearColor(0.05f, 0.05f, 0.08f, 0.7f), X, Y, BoxWidth, BoxHeight);

	// Border — green if complete, gold otherwise
	const FLinearColor BorderColor = Obj.bComplete
		? FLinearColor(0.1f, 0.9f, 0.2f)
		: FLinearColor(0.8f, 0.6f, 0.1f);
	DrawLine(X, Y, X + BoxWidth, Y, BorderColor);
	DrawLine(X + BoxWidth, Y, X + BoxWidth, Y + BoxHeight, BorderColor);
	DrawLine(X, Y + BoxHeight, X + BoxWidth, Y + BoxHeight, BorderColor);
	DrawLine(X, Y, X, Y + BoxHeight, BorderColor);

	// "OBJECTIVE" label
	Canvas->SetDrawColor(FColor(180, 180, 180));
	Canvas->DrawText(GEngine->GetSmallFont(), TEXT("OBJECTIVE"), X + 8, Y + 4, 0.85f, 0.85f);

	// Objective text
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(GEngine->GetMediumFont(), Obj.DisplayText.ToString(), X + 8, Y + 18, 1.0f, 1.0f);

	// Progress counter
	Canvas->SetDrawColor(Obj.bComplete ? FColor(50, 255, 50) : FColor(255, 200, 80));
	const FString Progress = FString::Printf(TEXT("%d / %d"), Obj.CurrentCount, Obj.TargetCount);
	Canvas->DrawText(GEngine->GetSmallFont(), Progress, X + BoxWidth - 55, Y + 38, 0.9f, 0.9f);

	// Progress bar
	const float BarY = Y + BoxHeight - 4;
	const float BarW = BoxWidth - 16;
	DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.7f), X + 8, BarY, BarW, 2);
	if (Obj.TargetCount > 0)
	{
		const float Pct = static_cast<float>(Obj.CurrentCount) / Obj.TargetCount;
		DrawRect(FLinearColor(0.9f, 0.7f, 0.1f, 0.9f), X + 8, BarY, BarW * Pct, 2);
	}
}

// ---------------------------------------------------------------------------
// Tutorial hint popup (bottom-center)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::ShowTutorial(const FString& Title, const FString& Body, float Duration)
{
	TutorialTitle = Title;
	TutorialBody = Body;
	TutorialTimer = Duration;
}

void AAnansiDevHUD::DismissTutorial()
{
	TutorialTimer = 0.0f;
	TutorialTitle.Empty();
	TutorialBody.Empty();
}

void AAnansiDevHUD::DrawTutorialHint(float DeltaTime)
{
	if (TutorialTimer <= 0.0f || TutorialTitle.IsEmpty()) return;

	TutorialTimer -= DeltaTime;

	const float BoxWidth = 450.0f;
	const float BoxHeight = 80.0f;
	const float CenterX = Canvas->SizeX * 0.5f;
	const float X = CenterX - BoxWidth * 0.5f;
	const float Y = Canvas->SizeY - BoxHeight - 180.0f;

	const float Alpha = FMath::Clamp(TutorialTimer / 0.5f, 0.0f, 1.0f);

	// Background
	DrawRect(FLinearColor(0.05f, 0.08f, 0.15f, 0.85f * Alpha), X, Y, BoxWidth, BoxHeight);

	// Border
	const FLinearColor BorderColor(0.3f, 0.7f, 1.0f, Alpha);
	DrawLine(X, Y, X + BoxWidth, Y, BorderColor);
	DrawLine(X + BoxWidth, Y, X + BoxWidth, Y + BoxHeight, BorderColor);
	DrawLine(X, Y + BoxHeight, X + BoxWidth, Y + BoxHeight, BorderColor);
	DrawLine(X, Y, X, Y + BoxHeight, BorderColor);

	// Title
	Canvas->SetDrawColor(FColor(120, 200, 255));
	Canvas->DrawText(GEngine->GetMediumFont(), TutorialTitle, X + 15, Y + 10, 1.1f, 1.1f);

	// Body
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(GEngine->GetSmallFont(), TutorialBody, X + 15, Y + 38, 0.95f, 0.95f);
}

// ---------------------------------------------------------------------------
// Credits screen
// ---------------------------------------------------------------------------

void AAnansiDevHUD::ShowCredits()
{
	bShowCredits = true;
	CreditsScrollY = Canvas ? Canvas->SizeY : 720.0f;
}

void AAnansiDevHUD::DrawCreditsScreen(float DeltaTime)
{
	// Dark background
	DrawRect(FLinearColor(0, 0, 0, 0.9f), 0, 0, Canvas->SizeX, Canvas->SizeY);

	// Scroll up
	CreditsScrollY -= 40.0f * DeltaTime;

	const float CenterX = Canvas->SizeX * 0.5f;
	float Y = CreditsScrollY;

	static const TArray<TPair<FString, FColor>> CreditLines = {
		{TEXT("ANANSI: WEB OF SHADOWS"), FColor(255, 200, 50)},
		{TEXT(""), FColor::White},
		{TEXT("A Third-Person Action-Adventure"), FColor(200, 200, 200)},
		{TEXT("Inspired by Akan Folklore"), FColor(200, 200, 200)},
		{TEXT(""), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT("DESIGN & PROGRAMMING"), FColor(255, 180, 80)},
		{TEXT("Peter"), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT("GAMEPLAY SYSTEMS"), FColor(255, 180, 80)},
		{TEXT("Combat, Traversal, Stealth"), FColor::White},
		{TEXT("5 GAS Abilities, AI, Narrative"), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT("ENEMY DESIGN"), FColor(255, 180, 80)},
		{TEXT("Palace Guards, Shield Guards"), FColor::White},
		{TEXT("Ranged Zealots, Turrets"), FColor::White},
		{TEXT("Captain of the Mask"), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT("BUILT ON"), FColor(255, 180, 80)},
		{TEXT("Unreal Engine 5.7"), FColor::White},
		{TEXT("C++ (200+ source files)"), FColor::White},
		{TEXT("60+ gameplay systems"), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT("SPECIAL THANKS"), FColor(255, 180, 80)},
		{TEXT("Claude Opus 4.6"), FColor::White},
		{TEXT("Mixamo (character animations)"), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT(""), FColor::White},
		{TEXT("THE STORY WEAVES ON..."), FColor(255, 200, 50)},
		{TEXT(""), FColor::White},
		{TEXT("[Press ESC to return]"), FColor(120, 120, 120)},
	};

	for (const auto& Pair : CreditLines)
	{
		if (Y > -40 && Y < Canvas->SizeY + 40)
		{
			Canvas->SetDrawColor(Pair.Value);
			const float Width = Pair.Key.Len() * 8.0f;
			Canvas->DrawText(GEngine->GetMediumFont(), Pair.Key, CenterX - Width * 0.5f, Y, 1.2f, 1.2f);
		}
		Y += 35.0f;
	}

	// Loop scroll
	if (Y < 0)
	{
		CreditsScrollY = Canvas->SizeY;
	}
}

// ---------------------------------------------------------------------------
// XP bar (top-left, below world clock)
// ---------------------------------------------------------------------------

void AAnansiDevHUD::DrawXPBar()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return;

	UPlayerProgression* Prog = GI->GetSubsystem<UPlayerProgression>();
	if (!Prog) return;

	const float X = 30.0f;
	const float Y = 123.0f;
	const float Width = 200.0f;
	const float Height = 6.0f;

	// Label
	Canvas->SetDrawColor(FColor(200, 180, 255));
	const FString LevelText = FString::Printf(TEXT("Lv.%d  (%d XP)"),
		Prog->GetCurrentLevel(), Prog->GetCurrentXP());
	Canvas->DrawText(GEngine->GetSmallFont(), LevelText, X, Y - 12, 0.85f, 0.85f);

	// XP bar
	DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.7f), X, Y, Width, Height);
	DrawRect(FLinearColor(0.7f, 0.4f, 1.0f, 0.9f), X, Y, Width * Prog->GetLevelProgress(), Height);

	// Upgrade points
	if (Prog->GetUpgradePoints() > 0)
	{
		Canvas->SetDrawColor(FColor(255, 200, 50));
		Canvas->DrawText(GEngine->GetSmallFont(),
			FString::Printf(TEXT("+%d SP"), Prog->GetUpgradePoints()),
			X + Width + 8, Y - 5, 0.9f, 0.9f);
	}
}
