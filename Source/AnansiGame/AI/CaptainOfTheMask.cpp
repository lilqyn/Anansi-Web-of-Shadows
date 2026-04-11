// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/CaptainOfTheMask.h"
#include "AI/EnemyGuard.h"
#include "AI/SimpleEnemyAI.h"
#include "AnansiGame.h"
#include "Engine/World.h"

ACaptainOfTheMask::ACaptainOfTheMask()
{
	BossDisplayName = NSLOCTEXT("Anansi", "CaptainBoss", "Captain of the Mask");

	MaxHealth = 500.0f;
	BaseDamage = 20.0f;
	StaggerThreshold = 100.0f;

	// 2-phase fight: phase 2 at 50%
	PhaseHealthThresholds.Empty();
	PhaseHealthThresholds.Add(0.5f);

	if (BossAI)
	{
		BossAI->AttackDamage = 20.0f;
		BossAI->AttackCooldown = 2.0f;
		BossAI->ChaseSpeed = 300.0f;
		BossAI->AttackRange = 250.0f;
	}
}

void ACaptainOfTheMask::BeginPlay()
{
	Super::BeginPlay();

	// Listen for health changes to trigger guard summons
	OnHealthChanged.AddDynamic(this, &ACaptainOfTheMask::OnHealthThresholdCheck);

	UE_LOG(LogAnansi, Log, TEXT("Captain of the Mask: Fight begins! HP=%.0f"), GetMaxHealth());
}

void ACaptainOfTheMask::OnPhaseTransition(int32 NewPhase)
{
	Super::OnPhaseTransition(NewPhase);

	if (NewPhase == 2)
	{
		UE_LOG(LogAnansi, Log, TEXT("Captain of the Mask: PHASE 2 — Mask removed! Faster and stronger."));

		// Phase 2: more aggressive
		if (BossAI)
		{
			BossAI->AttackCooldown = 1.2f;
			BossAI->AttackDamage = 30.0f;
			BossAI->ChaseSpeed = 450.0f;
		}
	}
}

void ACaptainOfTheMask::OnHealthThresholdCheck(float NewHealth, float InMaxHealth)
{
	const float Pct = InMaxHealth > 0 ? NewHealth / InMaxHealth : 0;

	// Summon guards at 75% HP
	if (Pct <= 0.75f && !bPhase1GuardsSummoned)
	{
		bPhase1GuardsSummoned = true;
		SummonGuards(Phase1GuardCount);
		UE_LOG(LogAnansi, Log, TEXT("Captain summons %d guards (Phase 1)"), Phase1GuardCount);
	}

	// Summon guards at 25% HP
	if (Pct <= 0.25f && !bPhase2GuardsSummoned)
	{
		bPhase2GuardsSummoned = true;
		SummonGuards(Phase2GuardCount);
		UE_LOG(LogAnansi, Log, TEXT("Captain summons %d guards (Phase 2)"), Phase2GuardCount);
	}
}

void ACaptainOfTheMask::SummonGuards(int32 Count)
{
	const FVector Center = GetActorLocation();

	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = (static_cast<float>(i) / Count) * 360.0f;
		const float Rad = FMath::DegreesToRadians(Angle);
		const FVector SpawnLoc = Center + FVector(FMath::Cos(Rad) * GuardSummonRadius,
			FMath::Sin(Rad) * GuardSummonRadius, 0.0f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		GetWorld()->SpawnActor<AEnemyGuard>(AEnemyGuard::StaticClass(), SpawnLoc,
			(Center - SpawnLoc).Rotation(), Params);
	}
}
