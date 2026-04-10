// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Combat/CombatComponent.h"
#include "Combat/ComboData.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind montage-end callback so we know when an attack finishes.
	if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
	{
		if (UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance())
		{
			AnimInstance->OnMontageEnded.AddDynamic(this, &UCombatComponent::OnAttackMontageEnded);
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// -- Combo decay --------------------------------------------------------
	if (ComboCount > 0)
	{
		TimeSinceLastHit += DeltaTime;

		if (TimeSinceLastHit >= ComboDecayDelay)
		{
			// Decay the counter gradually.
			const float DecayAmount = ComboDecayRate * DeltaTime;
			const int32 OldCombo = ComboCount;
			ComboCount = FMath::Max(0, ComboCount - FMath::CeilToInt(DecayAmount));

			if (ComboCount != OldCombo)
			{
				OnComboCountChanged.Broadcast(OldCombo, ComboCount);

				const EStyleRank NewRank = CalculateStyleRank(ComboCount);
				if (NewRank != CurrentStyleRank)
				{
					const EStyleRank OldRank = CurrentStyleRank;
					CurrentStyleRank = NewRank;
					OnStyleRankChanged.Broadcast(OldRank, CurrentStyleRank);
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Attack interface
// ---------------------------------------------------------------------------

void UCombatComponent::RequestLightAttack()
{
	if (!LightComboData || LightComboData->GetHitCount() == 0)
	{
		return;
	}

	if (bIsAttacking)
	{
		// Queue the next input — the chain window handler will pick it up.
		bNextAttackQueued = true;
		return;
	}

	// Start fresh or continue the chain.
	if (!bCanChainNextHit)
	{
		LightComboIndex = 0;
	}

	PlayComboHit(LightComboData, LightComboIndex);
}

void UCombatComponent::RequestHeavyAttack()
{
	if (!HeavyComboData || HeavyComboData->GetHitCount() == 0)
	{
		return;
	}

	if (bIsAttacking)
	{
		bNextAttackQueued = true;
		return;
	}

	if (!bCanChainNextHit)
	{
		HeavyComboIndex = 0;
	}

	PlayComboHit(HeavyComboData, HeavyComboIndex);
}

bool UCombatComponent::PlayComboHit(UComboChainData* ComboData, int32 Index)
{
	const FComboHitDefinition* HitDef = ComboData->GetHit(Index);
	if (!HitDef || !HitDef->Montage)
	{
		UE_LOG(LogAnansi, Warning, TEXT("CombatComponent: No montage for combo index %d"), Index);
		return false;
	}

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner)
	{
		return false;
	}

	UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}

	const float PlayRate = 1.0f;
	const float MontageLength = AnimInstance->Montage_Play(HitDef->Montage, PlayRate);

	if (MontageLength <= 0.0f)
	{
		return false;
	}

	// If the montage has a named section, jump to it.
	if (HitDef->MontageSection != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(HitDef->MontageSection, HitDef->Montage);
	}

	bIsAttacking = true;
	bCanChainNextHit = false;
	bNextAttackQueued = false;

	// Set up the chain window timer — after this delay, the player can chain
	// into the next hit if they buffer an input.
	const float ChainWindowStart = MontageLength * HitDef->CancelWindow.X;
	FTimerHandle ChainTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		ChainTimerHandle,
		[this, ComboData, Index]()
		{
			bCanChainNextHit = true;

			if (bNextAttackQueued)
			{
				bNextAttackQueued = false;
				bIsAttacking = false;

				const int32 NextIndex = Index + 1;
				if (NextIndex < ComboData->GetHitCount())
				{
					if (ComboData == LightComboData)
					{
						LightComboIndex = NextIndex;
					}
					else
					{
						HeavyComboIndex = NextIndex;
					}
					PlayComboHit(ComboData, NextIndex);
				}
				else
				{
					// Chain exhausted — reset to the beginning.
					LightComboIndex = 0;
					HeavyComboIndex = 0;
					bCanChainNextHit = false;
				}
			}
		},
		ChainWindowStart,
		false);

	UE_LOG(LogAnansi, Verbose, TEXT("Playing combo hit %d, montage: %s"), Index, *HitDef->Montage->GetName());
	return true;
}

void UCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Only reset attack state if we are not already chaining.
	if (bIsAttacking)
	{
		bIsAttacking = false;

		if (!bCanChainNextHit)
		{
			// Combo window expired — reset.
			LightComboIndex = 0;
			HeavyComboIndex = 0;
		}
	}
}

void UCombatComponent::OnWeaponHitDetected(const FVector& HitLocation, const FVector& HitNormal, AActor* HitTarget)
{
	if (!HitTarget)
	{
		return;
	}

	// Increment the combo counter.
	IncrementCombo();

	// Determine current hit definition for damage values.
	float BaseDamage = 10.0f;
	bool bShouldHitStop = false;
	float CurrentHitStopDuration = HitStopDuration;

	if (LightComboData)
	{
		const FComboHitDefinition* HitDef = LightComboData->GetHit(LightComboIndex);
		if (HitDef)
		{
			BaseDamage = HitDef->BaseDamage;
			bShouldHitStop = HitDef->bTriggerHitStop;
			CurrentHitStopDuration = HitDef->HitStopDuration;
		}
	}

	// Apply hit-stop for impact feel.
	if (bShouldHitStop)
	{
		ApplyHitStop(CurrentHitStopDuration, HitStopTimeDilation);
	}

	// Camera shake.
	if (HitCameraShake)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientStartCameraShake(HitCameraShake);
		}
	}

	UE_LOG(LogAnansi, Verbose, TEXT("Hit %s for %.1f damage"), *HitTarget->GetName(), BaseDamage);
}

// ---------------------------------------------------------------------------
// Defence
// ---------------------------------------------------------------------------

void UCombatComponent::StartDodge()
{
	if (bIsInvulnerable)
	{
		return;
	}

	bIsInvulnerable = true;

	// Play dodge montage if configured.
	if (DodgeMontage)
	{
		if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
		{
			if (UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_Play(DodgeMontage);
			}
		}
	}

	// Schedule i-frame end.
	GetWorld()->GetTimerManager().SetTimer(
		IFrameTimerHandle,
		this,
		&UCombatComponent::EndIFrames,
		DodgeIFrameDuration,
		false);
}

void UCombatComponent::EndIFrames()
{
	bIsInvulnerable = false;
}

void UCombatComponent::StartParry()
{
	if (bIsParrying)
	{
		return;
	}

	bIsParrying = true;

	if (ParryMontage)
	{
		if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
		{
			if (UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_Play(ParryMontage);
			}
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		ParryTimerHandle,
		this,
		&UCombatComponent::EndParry,
		ParryWindowDuration,
		false);
}

void UCombatComponent::EndParry()
{
	bIsParrying = false;
}

float UCombatComponent::ReceiveDamage(const FAnansiDamageInfo& DamageInfo)
{
	// Dodge i-frames negate all damage.
	if (bIsInvulnerable)
	{
		OnDodgeSuccess.Broadcast();
		return 0.0f;
	}

	// Successful parry negates damage and triggers counter.
	if (bIsParrying && !DamageInfo.bUnblockable)
	{
		EndParry();
		OnParrySuccess.Broadcast();

		// Play counter-attack montage if assigned.
		if (ParryCounterMontage)
		{
			if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
			{
				if (UAnimInstance* AnimInstance = Owner->GetMesh()->GetAnimInstance())
				{
					AnimInstance->Montage_Play(ParryCounterMontage);
				}
			}
		}
		return 0.0f;
	}

	// Damage goes through — reset combo.
	ResetCombo();

	// Camera shake on receiving damage.
	if (DamageCameraShake)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientStartCameraShake(DamageCameraShake);
		}
	}

	OnDamageReceived.Broadcast(DamageInfo);
	return DamageInfo.Amount;
}

// ---------------------------------------------------------------------------
// Lock-on
// ---------------------------------------------------------------------------

void UCombatComponent::ToggleLockOn()
{
	if (LockOnTarget.IsValid())
	{
		// Release current lock.
		LockOnTarget.Reset();
		return;
	}

	RefreshLockOnCandidates();

	if (LockOnCandidates.Num() > 0)
	{
		LockOnTarget = LockOnCandidates[0];
		UE_LOG(LogAnansi, Log, TEXT("Locked on to %s"), *LockOnTarget->GetName());
	}
}

void UCombatComponent::SwitchLockOnTarget(float Direction)
{
	if (!LockOnTarget.IsValid())
	{
		return;
	}

	RefreshLockOnCandidates();

	if (LockOnCandidates.Num() < 2)
	{
		return;
	}

	// Find current target index.
	int32 CurrentIndex = INDEX_NONE;
	for (int32 i = 0; i < LockOnCandidates.Num(); ++i)
	{
		if (LockOnCandidates[i] == LockOnTarget)
		{
			CurrentIndex = i;
			break;
		}
	}

	if (CurrentIndex == INDEX_NONE)
	{
		LockOnTarget = LockOnCandidates[0];
		return;
	}

	const int32 Step = (Direction >= 0.0f) ? 1 : -1;
	const int32 NewIndex = (CurrentIndex + Step + LockOnCandidates.Num()) % LockOnCandidates.Num();
	LockOnTarget = LockOnCandidates[NewIndex];
}

void UCombatComponent::RefreshLockOnCandidates()
{
	LockOnCandidates.Reset();

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector Origin = Owner->GetActorLocation();
	const FVector Forward = Owner->GetActorForwardVector();
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(LockOnConeHalfAngle));

	// Gather all actors with tag "Enemy" within range and cone.
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), AllActors);

	for (AActor* Candidate : AllActors)
	{
		if (!Candidate || Candidate == Owner)
		{
			continue;
		}

		const FVector ToTarget = Candidate->GetActorLocation() - Origin;
		const float Dist = ToTarget.Size();
		if (Dist > LockOnMaxDistance)
		{
			continue;
		}

		const FVector DirToTarget = ToTarget.GetSafeNormal();
		if (FVector::DotProduct(Forward, DirToTarget) < CosHalfAngle)
		{
			continue;
		}

		LockOnCandidates.Add(Candidate);
	}

	// Sort by distance for deterministic switching.
	LockOnCandidates.Sort([&Origin](const TWeakObjectPtr<AActor>& A, const TWeakObjectPtr<AActor>& B)
	{
		if (!A.IsValid() || !B.IsValid()) return false;
		return FVector::DistSquared(Origin, A->GetActorLocation()) < FVector::DistSquared(Origin, B->GetActorLocation());
	});
}

// ---------------------------------------------------------------------------
// Combo / Style
// ---------------------------------------------------------------------------

void UCombatComponent::IncrementCombo()
{
	const int32 OldCombo = ComboCount;
	ComboCount = FMath::Min(ComboCount + 1, AnansiConstants::MaxComboCount);
	TimeSinceLastHit = 0.0f;

	OnComboCountChanged.Broadcast(OldCombo, ComboCount);

	const EStyleRank NewRank = CalculateStyleRank(ComboCount);
	if (NewRank != CurrentStyleRank)
	{
		const EStyleRank OldRank = CurrentStyleRank;
		CurrentStyleRank = NewRank;
		OnStyleRankChanged.Broadcast(OldRank, CurrentStyleRank);
	}
}

void UCombatComponent::ResetCombo()
{
	if (ComboCount == 0)
	{
		return;
	}

	const int32 OldCombo = ComboCount;
	ComboCount = 0;
	TimeSinceLastHit = 0.0f;
	OnComboCountChanged.Broadcast(OldCombo, 0);

	if (CurrentStyleRank != EStyleRank::D)
	{
		const EStyleRank OldRank = CurrentStyleRank;
		CurrentStyleRank = EStyleRank::D;
		OnStyleRankChanged.Broadcast(OldRank, EStyleRank::D);
	}
}

EStyleRank UCombatComponent::CalculateStyleRank(int32 Combo) const
{
	// Thresholds tuned for feel — designers can adjust via subclass or data.
	if (Combo >= 100) return EStyleRank::SSS;
	if (Combo >= 70)  return EStyleRank::SS;
	if (Combo >= 50)  return EStyleRank::S;
	if (Combo >= 30)  return EStyleRank::A;
	if (Combo >= 15)  return EStyleRank::B;
	if (Combo >= 5)   return EStyleRank::C;
	return EStyleRank::D;
}

// ---------------------------------------------------------------------------
// Hit-stop
// ---------------------------------------------------------------------------

void UCombatComponent::ApplyHitStop(float Duration, float Dilation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Use custom time dilation on the owner rather than global time dilation
	// so that UI and other actors are unaffected.
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->CustomTimeDilation = Dilation;
	}

	World->GetTimerManager().ClearTimer(HitStopTimerHandle);
	World->GetTimerManager().SetTimer(
		HitStopTimerHandle,
		this,
		&UCombatComponent::EndHitStop,
		Duration,
		false);
}

void UCombatComponent::EndHitStop()
{
	if (AActor* Owner = GetOwner())
	{
		Owner->CustomTimeDilation = 1.0f;
	}
}
