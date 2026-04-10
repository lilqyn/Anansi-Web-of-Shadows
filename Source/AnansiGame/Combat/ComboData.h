// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Combat/DamageTypes.h"
#include "ComboData.generated.h"

class UAnimMontage;

/**
 * Describes a single hit within a combo chain.
 * Designers populate arrays of these inside a UComboChainData asset to define
 * the full light/heavy attack strings.
 */
USTRUCT(BlueprintType)
struct FComboHitDefinition
{
	GENERATED_BODY()

	/** Animation montage to play for this hit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	/** Section name within the montage (if using composite montages). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FName MontageSection = NAME_None;

	/** Base damage dealt by this hit (before multipliers). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	/** Damage type for this hit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	EAnansiDamageType DamageType = EAnansiDamageType::Physical;

	/**
	 * Angle (in degrees) of the hit arc relative to the character's forward.
	 * Used for hit-reaction selection on the target.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float HitArcAngle = 90.0f;

	/** Knockback / launch power applied on hit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0"))
	float LaunchPower = 200.0f;

	/**
	 * Time window (in seconds) after this hit's active frames end during which
	 * the player can input the next attack to continue the chain.
	 * If the window expires without input, the combo resets.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0"))
	float NextHitInputWindow = 0.4f;

	/**
	 * Normalised time range within the montage where the player is allowed to
	 * cancel into a dodge, jump, or special ability. X = start, Y = end.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FVector2D CancelWindow = FVector2D(0.5f, 1.0f);

	/** If true, this hit triggers hit-stop (brief time dilation) for impact feel. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	bool bTriggerHitStop = false;

	/** Duration of the hit-stop effect in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.0", EditCondition = "bTriggerHitStop"))
	float HitStopDuration = 0.05f;
};

/**
 * UComboChainData
 *
 * Data asset that defines a full combo chain (e.g., "Light Attack String",
 * "Heavy Attack String", "Air Combo"). Each chain is an ordered array of
 * FComboHitDefinition entries. The combat component walks through the array
 * as the player lands successive inputs within timing windows.
 */
UCLASS(BlueprintType)
class ANANSIGAME_API UComboChainData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Human-readable name shown in debug UI. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FText ChainDisplayName;

	/** Ordered list of hits in this combo chain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TArray<FComboHitDefinition> Hits;

	/** Get the number of hits in this chain. */
	UFUNCTION(BlueprintPure, Category = "Combo")
	int32 GetHitCount() const { return Hits.Num(); }

	/** Safely retrieve a hit definition by index. Returns nullptr if out of range. */
	const FComboHitDefinition* GetHit(int32 Index) const
	{
		return Hits.IsValidIndex(Index) ? &Hits[Index] : nullptr;
	}
};
