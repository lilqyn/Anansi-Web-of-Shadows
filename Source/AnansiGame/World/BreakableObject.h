// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakableObject.generated.h"

class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectBroken, ABreakableObject*, Object);

/**
 * ABreakableObject
 *
 * Destructible environmental object (crates, pots, barrels).
 * Takes damage, breaks when health reaches zero, drops loot.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ABreakableObject : public AActor
{
	GENERATED_BODY()

public:
	ABreakableObject();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Breakable", meta = (ClampMin = "1.0"))
	float MaxHealth = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Breakable")
	bool bDropsLoot = true;

	/** Scale of the object (uniform). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Breakable", meta = (ClampMin = "0.1"))
	float ObjectScale = 0.5f;

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Breakable")
	FOnObjectBroken OnBroken;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ObjectMesh;

private:
	float CurrentHealth;
	bool bIsBroken = false;

	void Break(AActor* Breaker);
};
