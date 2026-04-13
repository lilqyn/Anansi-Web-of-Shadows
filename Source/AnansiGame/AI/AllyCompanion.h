// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AllyCompanion.generated.h"

class UMeleeDamageDealer;

/**
 * AAllyCompanion
 *
 * Friendly AI that follows the player and attacks enemies.
 * Tagged as "Ally" so enemies target both the player and them.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AAllyCompanion : public ACharacter
{
	GENERATED_BODY()

public:
	AAllyCompanion();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Ally", meta = (ClampMin = "100.0"))
	float FollowDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Ally", meta = (ClampMin = "100.0"))
	float EngageRange = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Ally", meta = (ClampMin = "0.1"))
	float AttackCooldown = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Ally", meta = (ClampMin = "1.0"))
	float AttackDamage = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Ally", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMeleeDamageDealer> MeleeDamage;

private:
	float AttackTimer = 0.0f;
	float CurrentHealth;
	bool bIsDead = false;

	TWeakObjectPtr<AActor> CurrentTarget;

	void FindTarget();
	void FollowPlayer(float DeltaTime);
	void EngageTarget(float DeltaTime);
};
