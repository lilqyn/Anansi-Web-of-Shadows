// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleWall.generated.h"

class UStaticMeshComponent;

/**
 * ADestructibleWall
 *
 * Weak wall that blocks passage but can be broken by the player.
 * Requires heavy hits to destroy (higher HP than regular breakables).
 * When destroyed, reveals whatever is behind it.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ADestructibleWall : public AActor
{
	GENERATED_BODY()

public:
	ADestructibleWall();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Wall")
	float MaxHealth = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Wall")
	FVector WallScale = FVector(0.3f, 3.0f, 4.0f);

	/** Minimum damage per hit required (filters out small hits). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Wall")
	float MinDamageThreshold = 20.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> WallMesh;

private:
	float CurrentHealth;
	bool bIsBroken = false;

	void Break();
};
