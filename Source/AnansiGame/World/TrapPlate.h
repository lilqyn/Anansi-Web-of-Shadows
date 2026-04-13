// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrapPlate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ETrapType : uint8
{
	SpikeLaunch	UMETA(DisplayName = "Spike Launch (upward)"),
	SlowField	UMETA(DisplayName = "Slow Field"),
	DamagePulse	UMETA(DisplayName = "Damage Pulse")
};

/**
 * ATrapPlate
 *
 * Pressure plate that activates a trap when stepped on.
 * Affects both player and enemies.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ATrapPlate : public AActor
{
	GENERATED_BODY()

public:
	ATrapPlate();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Trap")
	ETrapType TrapType = ETrapType::SpikeLaunch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Trap", meta = (ClampMin = "0.0"))
	float TrapDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Trap", meta = (ClampMin = "0.5"))
	float ResetTime = 3.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> PlateTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PlateMesh;

	UFUNCTION()
	void OnSteppedOn(UPrimitiveComponent* Comp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bIsArmed = true;

	void ActivateTrap(ACharacter* Victim);
	void RearmTrap();
};
