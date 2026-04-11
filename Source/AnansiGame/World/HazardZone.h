// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HazardZone.generated.h"

class UBoxComponent;

UENUM(BlueprintType)
enum class EHazardType : uint8
{
	Fire	UMETA(DisplayName = "Fire"),
	Poison	UMETA(DisplayName = "Poison"),
	Spirit	UMETA(DisplayName = "Spirit")
};

/**
 * AHazardZone
 *
 * Environmental damage zone. Deals periodic damage to any character
 * that overlaps the volume. Fire deals burst damage, Poison slows
 * and damages over time, Spirit drains web energy.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AHazardZone : public AActor
{
	GENERATED_BODY()

public:
	AHazardZone();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Hazard")
	EHazardType HazardType = EHazardType::Fire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Hazard", meta = (ClampMin = "0.0"))
	float DamagePerSecond = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Hazard", meta = (ClampMin = "0.1"))
	float DamageTickInterval = 0.5f;

	/** Slow multiplier for Poison type (0.5 = half speed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Hazard",
		meta = (ClampMin = "0.1", ClampMax = "1.0", EditCondition = "HazardType == EHazardType::Poison"))
	float PoisonSlowMultiplier = 0.5f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Hazard")
	TObjectPtr<UBoxComponent> HazardVolume;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* Comp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* Comp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:
	TSet<TWeakObjectPtr<AActor>> OverlappingActors;
	float DamageTickTimer = 0.0f;
};
