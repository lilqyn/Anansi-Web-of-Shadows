// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyBase.h"
#include "EnemyRanged.generated.h"

class USimpleEnemyAI;

/**
 * AEnemyRanged
 *
 * Ranged attacker enemy — keeps distance from the player and fires
 * projectile-like damage at intervals. Weaker in melee range.
 * Retreats when the player gets too close.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AEnemyRanged : public AEnemyBase
{
	GENERATED_BODY()

public:
	AEnemyRanged();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Range at which this enemy prefers to fight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Ranged", meta = (ClampMin = "200.0"))
	float PreferredRange = 1000.0f;

	/** Damage per ranged shot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Ranged", meta = (ClampMin = "1.0"))
	float RangedDamage = 8.0f;

	/** Cooldown between shots. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Ranged", meta = (ClampMin = "0.5"))
	float ShotCooldown = 2.5f;

	/** Projectile speed (for visual trace). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Ranged", meta = (ClampMin = "500.0"))
	float ProjectileSpeed = 3000.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USimpleEnemyAI> RangedAI;

private:
	float ShotTimer = 0.0f;

	void FireAtTarget();
	void MaintainDistance(float DeltaTime);

	TWeakObjectPtr<AActor> Target;
};
