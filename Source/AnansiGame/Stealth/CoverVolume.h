// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoverVolume.generated.h"

class UBoxComponent;

/**
 * Cover type — determines visual treatment and stealth bonus.
 */
UENUM(BlueprintType)
enum class ECoverType : uint8
{
	Full		UMETA(DisplayName = "Full Cover"),
	Partial		UMETA(DisplayName = "Partial Cover"),
	Foliage		UMETA(DisplayName = "Foliage")
};

/**
 * ACoverVolume
 *
 * A placed volume in the world that grants a stealth bonus to the player
 * while overlapping. Used by the StealthComponent to modulate visibility.
 */
UCLASS()
class ANANSIGAME_API ACoverVolume : public AActor
{
	GENERATED_BODY()

public:
	ACoverVolume();

	// -------------------------------------------------------------------
	// Cover properties
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Cover")
	ECoverType GetCoverType() const { return CoverType; }

	UFUNCTION(BlueprintPure, Category = "Anansi|Cover")
	float GetStealthBonus() const { return StealthBonus; }

protected:
	virtual void BeginPlay() override;

	// -------------------------------------------------------------------
	// Configuration
	// -------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anansi|Cover")
	ECoverType CoverType = ECoverType::Full;

	/** Stealth bonus applied while the player is inside (0-1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anansi|Cover", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StealthBonus = 0.8f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Cover")
	TObjectPtr<UBoxComponent> CoverBox;

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
