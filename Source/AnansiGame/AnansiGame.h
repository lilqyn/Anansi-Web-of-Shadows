// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAnansi, Log, All);

/**
 * Shared constants and utility macros for the Anansi: Web of Shadows game module.
 */
namespace AnansiConstants
{
	/** Maximum combo counter before automatic reset. */
	constexpr int32 MaxComboCount = 999;

	/** Default web energy pool size. */
	constexpr float DefaultWebEnergy = 100.0f;

	/** Default stamina pool size. */
	constexpr float DefaultStamina = 100.0f;

	/** Default health pool size. */
	constexpr float DefaultHealth = 100.0f;

	/** Parry timing window in seconds. */
	constexpr float ParryWindowDuration = 0.2f;

	/** Dodge i-frame duration in seconds. */
	constexpr float DodgeIFrameDuration = 0.3f;
}
