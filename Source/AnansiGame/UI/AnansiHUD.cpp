// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "UI/AnansiHUD.h"
#include "AnansiGame.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UAnansiHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// Start with interaction prompt and spider sense hidden.
	SetInteractionPrompt(false);
	SetLockOnReticleVisible(false);

	if (SpiderSenseIndicator)
	{
		SpiderSenseIndicator->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UAnansiHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Fade out spider sense indicator.
	if (SpiderSenseFlashTimer > 0.0f)
	{
		SpiderSenseFlashTimer -= InDeltaTime;

		if (SpiderSenseIndicator)
		{
			const float Alpha = FMath::Clamp(SpiderSenseFlashTimer / SpiderSenseFlashDuration, 0.0f, 1.0f);
			SpiderSenseIndicator->SetRenderOpacity(Alpha);

			if (SpiderSenseFlashTimer <= 0.0f)
			{
				SpiderSenseIndicator->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

void UAnansiHUD::SetHealthPercent(float Percent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}
}

void UAnansiHUD::SetWebEnergyPercent(float Percent)
{
	if (WebEnergyBar)
	{
		WebEnergyBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}
}

void UAnansiHUD::SetStaminaPercent(float Percent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}
}

void UAnansiHUD::SetComboDisplay(int32 ComboCount, const FText& RankText)
{
	if (ComboCountText)
	{
		if (ComboCount > 0)
		{
			ComboCountText->SetText(FText::AsNumber(ComboCount));
			ComboCountText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			ComboCountText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (StyleRankText)
	{
		if (!RankText.IsEmpty() && ComboCount > 0)
		{
			StyleRankText->SetText(RankText);
			StyleRankText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			StyleRankText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UAnansiHUD::SetInteractionPrompt(bool bVisible, const FText& PromptText)
{
	if (InteractionPromptText)
	{
		if (bVisible && !PromptText.IsEmpty())
		{
			InteractionPromptText->SetText(PromptText);
			InteractionPromptText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			InteractionPromptText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UAnansiHUD::TriggerSpiderSenseWarning()
{
	SpiderSenseFlashTimer = SpiderSenseFlashDuration;

	if (SpiderSenseIndicator)
	{
		SpiderSenseIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);
		SpiderSenseIndicator->SetRenderOpacity(1.0f);
	}
}

void UAnansiHUD::SetLockOnReticleVisible(bool bVisible)
{
	if (LockOnReticle)
	{
		LockOnReticle->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
