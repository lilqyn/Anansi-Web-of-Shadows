// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/AdinkraPuzzle.h"
#include "AnansiGame.h"

AAdinkraPuzzle::AAdinkraPuzzle()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Default solution: Sankofa -> GYeNyame -> Nyansapo
	Solution.Add(EAdinkraSymbol::Sankofa);
	Solution.Add(EAdinkraSymbol::GYeNyame);
	Solution.Add(EAdinkraSymbol::Nyansapo);
}

void AAdinkraPuzzle::SubmitSymbol(EAdinkraSymbol Symbol)
{
	if (bIsSolved)
	{
		return;
	}

	OnSymbolActivated.Broadcast(Symbol);

	if (!Solution.IsValidIndex(CurrentIndex))
	{
		return;
	}

	if (Solution[CurrentIndex] == Symbol)
	{
		CurrentIndex++;
		UE_LOG(LogAnansi, Log, TEXT("Puzzle: Correct symbol! Progress %d/%d"), CurrentIndex, Solution.Num());

		if (CurrentIndex >= Solution.Num())
		{
			bIsSolved = true;
			OnPuzzleSolved.Broadcast();
			UE_LOG(LogAnansi, Log, TEXT("Puzzle SOLVED!"));
		}
	}
	else
	{
		UE_LOG(LogAnansi, Log, TEXT("Puzzle: Wrong symbol — resetting."));
		ResetPuzzle();
	}
}

void AAdinkraPuzzle::ResetPuzzle()
{
	CurrentIndex = 0;
	bIsSolved = false;
	OnPuzzleReset.Broadcast();
}
