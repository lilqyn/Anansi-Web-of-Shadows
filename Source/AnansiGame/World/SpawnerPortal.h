// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnerPortal.generated.h"

class UStaticMeshComponent;

/**
 * ASpawnerPortal
 *
 * Continuously spawns enemies at a set interval until destroyed
 * or a kill limit is reached. Visual spinning portal effect.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ASpawnerPortal : public AActor
{
	GENERATED_BODY()

public:
	ASpawnerPortal();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Spawner")
	TSubclassOf<APawn> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Spawner", meta = (ClampMin = "1.0"))
	float SpawnInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Spawner", meta = (ClampMin = "1"))
	int32 MaxSpawns = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Spawner", meta = (ClampMin = "1.0"))
	float PortalHealth = 100.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PortalMesh;

private:
	float SpawnTimer = 0.0f;
	int32 SpawnCount = 0;
	float CurrentHealth;
	bool bDestroyed = false;

	void SpawnEnemy();
};
