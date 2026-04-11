// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/BossBase.h"
#include "AI/SimpleEnemyAI.h"
#include "AnansiGame.h"
#include "Engine/DamageEvents.h"

ABossBase::ABossBase()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxHealth = 500.0f;
	BaseDamage = 20.0f;
	StaggerThreshold = 80.0f;
	bRagdollOnDeath = false;

	BossAI = CreateDefaultSubobject<USimpleEnemyAI>(TEXT("BossAI"));
	BossAI->AttackDamage = BaseDamage;
	BossAI->AttackCooldown = 2.5f;
	BossAI->ChaseSpeed = 350.0f;
	BossAI->DetectionRange = 3000.0f;
	BossAI->AttackRange = 250.0f;

	// Default 2-phase boss (phase 2 at 50% HP)
	PhaseHealthThresholds.Add(0.5f);

	Tags.Add(FName("Boss"));
}

void ABossBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentPhase = 1;
}

float ABossBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float PrevHealthPct = GetHealthPercent();
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float NewHealthPct = GetHealthPercent();

	// Check for phase transitions
	for (int32 i = 0; i < PhaseHealthThresholds.Num(); ++i)
	{
		const int32 PhaseNumber = i + 2; // Phase 2, 3, etc.
		const float Threshold = PhaseHealthThresholds[i];

		if (PrevHealthPct > Threshold && NewHealthPct <= Threshold && CurrentPhase < PhaseNumber)
		{
			OnPhaseTransition(PhaseNumber);
			break;
		}
	}

	// Boss defeated
	if (IsDead())
	{
		OnBossDefeated.Broadcast();
		UE_LOG(LogAnansi, Log, TEXT("Boss %s defeated!"), *BossDisplayName.ToString());
	}

	return ActualDamage;
}

void ABossBase::OnPhaseTransition(int32 NewPhase)
{
	CurrentPhase = NewPhase;
	OnBossPhaseChanged.Broadcast(NewPhase);

	UE_LOG(LogAnansi, Log, TEXT("Boss %s entering phase %d"), *BossDisplayName.ToString(), NewPhase);

	// Phase 2+: increase aggression
	if (BossAI)
	{
		BossAI->AttackCooldown = FMath::Max(0.5f, BossAI->AttackCooldown - 0.5f);
		BossAI->AttackDamage *= 1.3f;
		BossAI->ChaseSpeed += 50.0f;
	}
}
