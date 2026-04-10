// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Narrative/ChoiceManager.h"
#include "AnansiGame.h"

void UChoiceManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeTrustMeters();
	UE_LOG(LogAnansi, Log, TEXT("ChoiceManager initialized"));
}

void UChoiceManager::InitializeTrustMeters()
{
	TrustMeters.Add(ETrustTarget::AmaSerwaa, 0.0f);
	TrustMeters.Add(ETrustTarget::Anansi, 0.0f);
	TrustMeters.Add(ETrustTarget::VillageElders, 0.0f);
	TrustMeters.Add(ETrustTarget::SpiritRealm, 0.0f);
	TrustMeters.Add(ETrustTarget::Trickster, 0.0f);
}

// ---------------------------------------------------------------------------
// Choice application
// ---------------------------------------------------------------------------

void UChoiceManager::ApplyChoiceConsequence(FName ConsequenceKey)
{
	if (ConsequenceKey.IsNone())
	{
		return;
	}

	const FChoiceConsequence* Consequence = ConsequenceDatabase.Find(ConsequenceKey);
	if (!Consequence)
	{
		UE_LOG(LogAnansi, Warning, TEXT("Consequence key '%s' not found in database"), *ConsequenceKey.ToString());
		return;
	}

	ApplyConsequence(*Consequence);
}

void UChoiceManager::ApplyConsequence(const FChoiceConsequence& Consequence)
{
	// Apply trust change
	if (!FMath::IsNearlyZero(Consequence.TrustDelta))
	{
		ModifyTrust(Consequence.Target, Consequence.TrustDelta);
	}

	// Set world state flag
	if (!Consequence.WorldStateFlag.IsNone())
	{
		SetWorldStateFlag(Consequence.WorldStateFlag, Consequence.WorldStateFlagValue);
	}
}

// ---------------------------------------------------------------------------
// Trust meters
// ---------------------------------------------------------------------------

float UChoiceManager::GetTrust(ETrustTarget Target) const
{
	const float* Value = TrustMeters.Find(Target);
	return Value ? *Value : 0.0f;
}

void UChoiceManager::ModifyTrust(ETrustTarget Target, float Delta)
{
	float& Trust = TrustMeters.FindOrAdd(Target, 0.0f);
	Trust = FMath::Clamp(Trust + Delta, -100.0f, 100.0f);

	OnTrustChanged.Broadcast(Target, Trust, Delta);

	UE_LOG(LogAnansi, Log, TEXT("Trust [%d] changed by %.1f -> %.1f"),
		static_cast<int32>(Target), Delta, Trust);
}

// ---------------------------------------------------------------------------
// World state flags
// ---------------------------------------------------------------------------

void UChoiceManager::SetWorldStateFlag(FName FlagName, int32 Value)
{
	WorldStateFlags.FindOrAdd(FlagName) = Value;
	OnWorldStateChanged.Broadcast(FlagName);

	UE_LOG(LogAnansi, Log, TEXT("World state flag '%s' = %d"), *FlagName.ToString(), Value);
}

int32 UChoiceManager::GetWorldStateFlag(FName FlagName) const
{
	const int32* Value = WorldStateFlags.Find(FlagName);
	return Value ? *Value : 0;
}

bool UChoiceManager::IsWorldStateFlagSet(FName FlagName) const
{
	return GetWorldStateFlag(FlagName) != 0;
}

// ---------------------------------------------------------------------------
// Ending determination
// ---------------------------------------------------------------------------

EGameEnding UChoiceManager::DetermineEnding() const
{
	const float AmaTrust = GetTrust(ETrustTarget::AmaSerwaa);
	const float AnansiTrust = GetTrust(ETrustTarget::Anansi);
	const float EldersTrust = GetTrust(ETrustTarget::VillageElders);
	const float SpiritTrust = GetTrust(ETrustTarget::SpiritRealm);
	const float TricksterTrust = GetTrust(ETrustTarget::Trickster);

	// Check for exile — all factions hostile
	const bool bAllNegative = AmaTrust < -30.0f && AnansiTrust < -30.0f &&
		EldersTrust < -30.0f && SpiritTrust < -30.0f;
	if (bAllNegative)
	{
		return EGameEnding::Exile;
	}

	// Unity — high trust with Ama and Elders
	if (AmaTrust > 60.0f && EldersTrust > 60.0f)
	{
		return EGameEnding::Unity;
	}

	// Spirit Bound — high spirit trust
	if (SpiritTrust > 60.0f && SpiritTrust > TricksterTrust)
	{
		return EGameEnding::SpiritBound;
	}

	// Trickster Legacy — high trickster/Anansi trust
	if (TricksterTrust > 60.0f || (AnansiTrust > 60.0f && TricksterTrust > 30.0f))
	{
		return EGameEnding::TricksterLegacy;
	}

	// Balance — moderate trust across the board
	const float AverageTrust = (AmaTrust + AnansiTrust + EldersTrust + SpiritTrust) / 4.0f;
	if (AverageTrust > 20.0f)
	{
		return EGameEnding::Balance;
	}

	return EGameEnding::Undetermined;
}

// ---------------------------------------------------------------------------
// Save / Load
// ---------------------------------------------------------------------------

void UChoiceManager::LoadFromSave(const TMap<ETrustTarget, float>& SavedTrust,
	const TMap<FName, int32>& SavedFlags)
{
	TrustMeters = SavedTrust;
	WorldStateFlags = SavedFlags;

	// Ensure all targets exist
	for (int32 i = 0; i <= static_cast<int32>(ETrustTarget::Trickster); ++i)
	{
		const ETrustTarget Target = static_cast<ETrustTarget>(i);
		if (!TrustMeters.Contains(Target))
		{
			TrustMeters.Add(Target, 0.0f);
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("Loaded choice data: %d trust entries, %d world flags"),
		TrustMeters.Num(), WorldStateFlags.Num());
}
