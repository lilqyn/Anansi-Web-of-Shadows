// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

/**
 * AAnansiProjectile
 *
 * Physics-driven projectile used by ranged enemies. Travels in a
 * direction, deals damage on impact, and destroys itself.
 * Visible tracer trail via debug draw (replaced by Niagara in production).
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AAnansiProjectile : public AActor
{
	GENERATED_BODY()

public:
	AAnansiProjectile();

	/** Fire the projectile in a direction. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Projectile")
	void Fire(FVector Direction, float Speed = 2000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Projectile", meta = (ClampMin = "1.0"))
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Projectile", meta = (ClampMin = "0.1"))
	float LifeTime = 5.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void Tick(float DeltaTime) override;

private:
	FVector PreviousLocation;
};
