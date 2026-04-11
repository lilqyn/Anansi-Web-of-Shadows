// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdinkraPuzzle.generated.h"

class USphereComponent;

/**
 * Adinkra symbol types used in puzzles.
 */
UENUM(BlueprintType)
enum class EAdinkraSymbol : uint8
{
	Sankofa		UMETA(DisplayName = "Sankofa - Return and get it"),
	GYeNyame	UMETA(DisplayName = "Gye Nyame - Except God"),
	Dwennimmen	UMETA(DisplayName = "Dwennimmen - Ram's Horns"),
	Adinkrahene	UMETA(DisplayName = "Adinkrahene - King of Symbols"),
	Nyansapo	UMETA(DisplayName = "Nyansapo - Wisdom Knot")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleSolved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSymbolActivated, EAdinkraSymbol, Symbol);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleReset);

/**
 * AAdinkraPuzzle
 *
 * Sequence-based puzzle where the player must activate Adinkra symbol
 * pillars in the correct order. Each pillar represents a symbol;
 * interacting with them in the right sequence solves the puzzle.
 *
 * Place one AAdinkraPuzzle manager and several AAdinkraPillar children
 * in the level. Configure the solution sequence on the manager.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AAdinkraPuzzle : public AActor
{
	GENERATED_BODY()

public:
	AAdinkraPuzzle();

	/** Submit a symbol to the puzzle sequence. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Puzzle")
	void SubmitSymbol(EAdinkraSymbol Symbol);

	/** Reset the puzzle to its initial state. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Puzzle")
	void ResetPuzzle();

	UFUNCTION(BlueprintPure, Category = "Anansi|Puzzle")
	bool IsSolved() const { return bIsSolved; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Puzzle")
	int32 GetCurrentProgress() const { return CurrentIndex; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Puzzle")
	int32 GetSolutionLength() const { return Solution.Num(); }

	// -- Configuration -------------------------------------------------------

	/** The correct sequence of symbols to solve the puzzle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Puzzle")
	TArray<EAdinkraSymbol> Solution;

	// -- Delegates ------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Puzzle")
	FOnPuzzleSolved OnPuzzleSolved;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Puzzle")
	FOnSymbolActivated OnSymbolActivated;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Puzzle")
	FOnPuzzleReset OnPuzzleReset;

private:
	int32 CurrentIndex = 0;
	bool bIsSolved = false;
};
