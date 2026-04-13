// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Combat/MeleeDamageDealer.h"
#include "AnansiGame.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Player/AnansiCharacter.h"
#include "Core/CombatStats.h"
#include "Core/SoundManager.h"

UMeleeDamageDealer::UMeleeDamageDealer()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UMeleeDamageDealer::FireAttack(float DamageOverride)
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return 0;
	}

	const float Damage = (DamageOverride >= 0.0f) ? DamageOverride : BaseDamage;

	const FVector Start = Owner->GetActorLocation();
	const FVector Forward = Owner->GetActorForwardVector();
	const FVector End = Start + Forward * AttackRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	Params.bTraceComplex = false;

	TArray<FHitResult> Hits;
	const FCollisionShape Shape = FCollisionShape::MakeSphere(AttackRadius);

	GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Shape, Params);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugCapsule(GetWorld(), (Start + End) * 0.5f, AttackRange * 0.5f, AttackRadius,
			FQuat::FindBetweenNormals(FVector::UpVector, Forward),
			Hits.Num() > 0 ? FColor::Red : FColor::Green, false, 0.3f);
	}
#endif

	int32 HitCount = 0;
	TSet<AActor*> AlreadyHit;

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == Owner || AlreadyHit.Contains(HitActor))
		{
			continue;
		}

		AlreadyHit.Add(HitActor);

		// Apply UE damage
		FDamageEvent DamageEvent;
		HitActor->TakeDamage(Damage, DamageEvent, Owner->GetInstigatorController(), Owner);

		// Knockback
		if (ACharacter* HitChar = Cast<ACharacter>(HitActor))
		{
			const FVector KnockDir = (HitActor->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
			HitChar->LaunchCharacter(KnockDir * KnockbackForce, true, false);
		}

		HitCount++;

		// Play hit sound
		if (USoundManager* SM = GetWorld()->GetSubsystem<USoundManager>())
		{
			SM->PlayHitSound(Hit.ImpactPoint);
		}

		// Restore small amount of web energy per hit
		if (AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(Owner))
		{
			Anansi->RestoreWebEnergy(3.0f);
		}

		// Track in combat stats
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			if (UCombatStatsSubsystem* Stats = GI->GetSubsystem<UCombatStatsSubsystem>())
			{
				Stats->RecordHit(Damage);
			}
		}

		UE_LOG(LogAnansi, Verbose, TEXT("MeleeDamageDealer: Hit %s for %.1f damage"), *HitActor->GetName(), Damage);
	}

	return HitCount;
}
