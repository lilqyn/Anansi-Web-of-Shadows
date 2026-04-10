// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ChoiceManager.generated.h"

/**
 * Named NPCs whose trust the player can build or lose.
 */
UENUM(BlueprintType)
enum class ETrustTarget : uint8
{
	AmaSerwaa		UMETA(DisplayName = "Ama Serwaa"),
	Anansi			UMETA(DisplayName = "Anansi"),
	VillageElders	UMETA(DisplayName = "Village Elders"),
	SpiritRealm		UMETA(DisplayName = "Spirit Realm"),
	Trickster		UMETA(DisplayName = "Trickster Path")
};

/**
 * Possible ending states, determined by accumulated choices.
 */
UENUM(BlueprintType)
enum class EGameEnding : uint8
{
	/** Player united the community and earned universal trust. */
	Unity			UMETA(DisplayName = "Unity"),

	/** Player served the spirit realm faithfully. */
	SpiritBound		UMETA(DisplayName = "Spirit Bound"),

	/** Player followed Anansi's trickster path. */
	TricksterLegacy	UMETA(DisplayName = "Trickster Legacy"),

	/** Player balanced all factions. */
	Balance			UMETA(DisplayName = "Balance"),

	/** Player lost trust everywhere. */
	Exile			UMETA(DisplayName = "Exile"),

	/** Not yet determined. */
	Undetermined	UMETA(DisplayName = "Undetermined")
};

/**
 * A single choice consequence entry.
 */
USTRUCT(BlueprintType)
struct FChoiceConsequence
{
	GENERATED_BODY()

	/** Which trust meter is affected. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ETrustTarget Target = ETrustTarget::AmaSerwaa;

	/** Amount to add (positive) or subtract (negative). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TrustDelta = 0.0f;

	/** World-state flag to set (empty = none). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName WorldStateFlag;

	/** Value for the world-state flag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 WorldStateFlagValue = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTrustChanged, ETrustTarget, Target, float, NewTrust, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldStateChanged, FName, FlagName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndingDetermined, EGameEnding, Ending);

/**
 * UChoiceManager
 *
 * Game-instance subsystem that tracks player choices, maintains trust meters
 * for key NPCs/factions, manages world-state flags, and determines which
 * ending the player will receive.
 */
UCLASS()
class ANANSIGAME_API UChoiceManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// -------------------------------------------------------------------
	// Choice application
	// -------------------------------------------------------------------

	/** Apply a choice consequence by key (looked up from the database). */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Choices")
	void ApplyChoiceConsequence(FName ConsequenceKey);

	/** Directly apply a consequence struct. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Choices")
	void ApplyConsequence(const FChoiceConsequence& Consequence);

	// -------------------------------------------------------------------
	// Trust meters
	// -------------------------------------------------------------------

	/** Get the current trust value for a target (-100 to 100). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Choices")
	float GetTrust(ETrustTarget Target) const;

	/** Directly modify a trust value. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Choices")
	void ModifyTrust(ETrustTarget Target, float Delta);

	/** Get all trust values as a map. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Choices")
	TMap<ETrustTarget, float> GetAllTrustValues() const { return TrustMeters; }

	// -------------------------------------------------------------------
	// World state flags
	// -------------------------------------------------------------------

	/** Set a world-state flag. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Choices")
	void SetWorldStateFlag(FName FlagName, int32 Value = 1);

	/** Get a world-state flag value (0 if unset). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Choices")
	int32 GetWorldStateFlag(FName FlagName) const;

	/** Check if a world-state flag is set (non-zero). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Choices")
	bool IsWorldStateFlagSet(FName FlagName) const;

	// -------------------------------------------------------------------
	// Ending determination
	// -------------------------------------------------------------------

	/** Evaluate accumulated choices and return the projected ending. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Choices")
	EGameEnding DetermineEnding() const;

	// -------------------------------------------------------------------
	// Save / Load
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Anansi|Choices")
	void LoadFromSave(const TMap<ETrustTarget, float>& SavedTrust, const TMap<FName, int32>& SavedFlags);

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Choices")
	FOnTrustChanged OnTrustChanged;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Choices")
	FOnWorldStateChanged OnWorldStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Choices")
	FOnEndingDetermined OnEndingDetermined;

protected:
	/** Consequence database — maps consequence keys to arrays of consequences. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Choices")
	TMap<FName, FChoiceConsequence> ConsequenceDatabase;

private:
	/** Trust meters for each target. Clamped -100 to 100. */
	UPROPERTY()
	TMap<ETrustTarget, float> TrustMeters;

	/** World-state flags. */
	UPROPERTY()
	TMap<FName, int32> WorldStateFlags;

	/** Initialize trust meters to default values. */
	void InitializeTrustMeters();
};
