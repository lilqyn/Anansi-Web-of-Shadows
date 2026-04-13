// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyBase.h"
#include "EnemyTurret.generated.h"

/**
 * AEnemyTurret
 *
 * Stationary enemy that auto-fires projectiles at the player when
 * in range and line of sight. Cannot move or be staggered easily.
 * Must be destroyed by attacking it directly.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AEnemyTurret : public AEnemyBase
{
	GENERATED_BODY()

public:
	AEnemyTurret();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Turret", meta = (ClampMin = "200.0"))
	float DetectionRange = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Turret", meta = (ClampMin = "0.5"))
	float FireRate = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Turret", meta = (ClampMin = "1.0"))
	float ProjectileDamage = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Turret", meta = (ClampMin = "500.0"))
	float ProjectileSpeed = 2500.0f;

	/** Rotation speed towards target (degrees/sec). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Turret", meta = (ClampMin = "10.0"))
	float TurnSpeed = 120.0f;

private:
	float FireTimer = 0.0f;

	TWeakObjectPtr<AActor> Target;

	void FindTarget();
	void AimAtTarget(float DeltaTime);
	void Fire();
	bool HasLineOfSight() const;
};
