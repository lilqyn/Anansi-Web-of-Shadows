// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/EnemyBase.h"
#include "AnansiGame.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Core/CombatStats.h"
#include "World/LootDrop.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
	Tags.Add(FName("Enemy"));
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	// Add type-specific tag
	switch (EnemyType)
	{
	case EEnemyType::Spirit:
		Tags.AddUnique(FName("Spirit"));
		break;
	case EEnemyType::Shade:
		Tags.AddUnique(FName("Spirit"));
		Tags.AddUnique(FName("Shade"));
		break;
	default:
		break;
	}
}

// ---------------------------------------------------------------------------
// Damage
// ---------------------------------------------------------------------------

float AEnemyBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	UE_LOG(LogAnansi, Verbose, TEXT("%s took %.1f damage (%.1f / %.1f)"),
		*GetName(), ActualDamage, CurrentHealth, MaxHealth);

	// Stagger check
	AccumulatedDamage += ActualDamage;
	if (AccumulatedDamage >= StaggerThreshold && !bIsDead)
	{
		AccumulatedDamage = 0.0f;

		FVector HitDir = FVector::ZeroVector;
		if (DamageCauser)
		{
			HitDir = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
		}
		PlayHitReaction(HitDir);
	}

	// Hit flash
	FlashHitColor();

	// Death check
	if (CurrentHealth <= 0.0f)
	{
		HandleDeath(EventInstigator);
	}

	// Update awareness on taking damage
	if (!bIsDead && AwarenessState != EEnemyAwareness::Combat)
	{
		SetAwarenessState(EEnemyAwareness::Combat);
	}

	return ActualDamage;
}

// ---------------------------------------------------------------------------
// Awareness
// ---------------------------------------------------------------------------

void AEnemyBase::SetAwarenessState(EEnemyAwareness NewState)
{
	if (AwarenessState == NewState)
	{
		return;
	}

	const EEnemyAwareness OldState = AwarenessState;
	AwarenessState = NewState;
	OnAwarenessChanged.Broadcast(OldState, NewState);
}

// ---------------------------------------------------------------------------
// Damage responses
// ---------------------------------------------------------------------------

void AEnemyBase::PlayHitReaction(const FVector& HitDirection)
{
	if (HitReactionMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(HitReactionMontage);
	}

	// Small knockback from hit
	ApplyKnockback(HitDirection, 300.0f);
}

void AEnemyBase::ApplyStagger(float Duration)
{
	if (bIsDead)
	{
		return;
	}

	if (StaggerMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(StaggerMontage);
	}

	// Disable movement during stagger
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();

		GetWorld()->GetTimerManager().SetTimer(StaggerTimerHandle, [this]()
		{
			if (!bIsDead && GetCharacterMovement())
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			}
		}, Duration, false);
	}
}

void AEnemyBase::ApplyKnockback(const FVector& Direction, float Force)
{
	if (bIsDead)
	{
		return;
	}

	LaunchCharacter(Direction * Force, true, false);
}

// ---------------------------------------------------------------------------
// Death
// ---------------------------------------------------------------------------

void AEnemyBase::HandleDeath(AController* Killer)
{
	bIsDead = true;
	OnDied.Broadcast(this);

	// Track kill in combat stats
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UCombatStatsSubsystem* Stats = GI->GetSubsystem<UCombatStatsSubsystem>())
		{
			Stats->RecordKill();
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("%s died"), *GetName());

	// Death VFX
	if (DeathVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), DeathVFX, GetActorLocation(), GetActorRotation(),
			FVector::OneVector, true, true, ENCPoolMethod::AutoRelease);
	}

	// Death sound
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation());
	}

	// Loot
	SpawnLoot();

	// Ragdoll or destroy
	if (bRagdollOnDeath)
	{
		EnableRagdoll();
		SetLifeSpan(10.0f);
	}
	else
	{
		SetLifeSpan(0.1f);
	}

	// Stop AI
	if (AController* AIController = GetController())
	{
		AIController->UnPossess();
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyBase::SpawnLoot()
{
	// Always drop a random loot orb
	ALootDrop::SpawnRandomLoot(this, GetActorLocation());

	// Also spawn designer-configured loot if set
	if (LootDropClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AActor>(LootDropClass, GetActorLocation() + FVector(0.0f, 0.0f, 50.0f),
			FRotator::ZeroRotator, SpawnParams);
	}
}

void AEnemyBase::FlashHitColor()
{
	// Use custom depth stencil to indicate hit — visible as a color change
	// when post-process or the HUD is set up to render custom depth.
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetRenderCustomDepth(true);
		MeshComp->SetCustomDepthStencilValue(255);
	}

	// Reset after a brief flash
	GetWorld()->GetTimerManager().ClearTimer(HitFlashTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(HitFlashTimerHandle, this,
		&AEnemyBase::ResetHitColor, 0.12f, false);
}

void AEnemyBase::ResetHitColor()
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetRenderCustomDepth(false);
		MeshComp->SetCustomDepthStencilValue(0);
	}
}

void AEnemyBase::EnableRagdoll()
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetComponentTickEnabled(false);
	}
}
