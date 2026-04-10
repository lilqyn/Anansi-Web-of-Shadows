// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Abilities/GA_SilkenPath.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UGA_SilkenPath::UGA_SilkenPath()
{
	EnergyCost = 20.0f;
	CooldownDuration = 1.5f;
}

void UGA_SilkenPath::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FHitResult Hit;
	if (!PerformCameraLineTrace(WebTraceDistance, Hit))
	{
		UE_LOG(LogAnansi, Verbose, TEXT("SilkenPath — line trace missed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UWorld* World = OwnerChar->GetWorld();
	const FVector PlayerLocation = OwnerChar->GetActorLocation();

	// Spawn VFX along the web thread
	SpawnWebThreadEffect(PlayerLocation, Hit.ImpactPoint, World);

	// Determine what we hit
	AActor* HitActor = Hit.GetActor();
	const bool bHitEnemy = HitActor && HitActor->ActorHasTag(TEXT("Enemy"));

	if (bHitEnemy)
	{
		PullEnemy(HitActor, PlayerLocation);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
	else
	{
		// Hit world geometry — spawn a web bridge
		SpawnWebBridge(PlayerLocation, Hit.ImpactPoint, World);

		// Destroy bridge after duration
		if (SpawnedBridge)
		{
			World->GetTimerManager().SetTimer(BridgeTimerHandle, [this, Handle, ActorInfo, ActivationInfo]()
			{
				if (SpawnedBridge)
				{
					SpawnedBridge->Destroy();
					SpawnedBridge = nullptr;
				}
				EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			}, BridgeDuration, false);
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
	}
}

void UGA_SilkenPath::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (const ACharacter* OwnerChar = GetOwnerCharacter())
	{
		OwnerChar->GetWorld()->GetTimerManager().ClearTimer(BridgeTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------

void UGA_SilkenPath::SpawnWebBridge(const FVector& Start, const FVector& End, UWorld* World)
{
	if (!WebBridgeClass || !World)
	{
		UE_LOG(LogAnansi, Warning, TEXT("SilkenPath — WebBridgeClass not set"));
		return;
	}

	const FVector MidPoint = (Start + End) * 0.5f;
	const FRotator Rotation = (End - Start).Rotation();
	const float Length = FVector::Dist(Start, End);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedBridge = World->SpawnActor<AActor>(WebBridgeClass, MidPoint, Rotation, SpawnParams);

	if (SpawnedBridge)
	{
		// Scale the bridge to span the distance (assumes default 100-unit mesh along X)
		const float ScaleFactor = Length / 100.0f;
		SpawnedBridge->SetActorScale3D(FVector(ScaleFactor, 1.0f, 1.0f));

		UE_LOG(LogAnansi, Log, TEXT("SilkenPath — spawned web bridge (%.0f units)"), Length);
	}
}

void UGA_SilkenPath::PullEnemy(AActor* Enemy, const FVector& PlayerLocation)
{
	if (!Enemy)
	{
		return;
	}

	ACharacter* EnemyChar = Cast<ACharacter>(Enemy);
	if (EnemyChar && EnemyChar->GetCharacterMovement())
	{
		const FVector Direction = (PlayerLocation - Enemy->GetActorLocation()).GetSafeNormal();
		EnemyChar->LaunchCharacter(Direction * EnemyPullSpeed, true, true);
		UE_LOG(LogAnansi, Log, TEXT("SilkenPath — pulling enemy %s"), *Enemy->GetName());
	}
}

void UGA_SilkenPath::SpawnWebThreadEffect(const FVector& Start, const FVector& End, UWorld* World)
{
	if (!WebThreadVFX || !World)
	{
		return;
	}

	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World, WebThreadVFX, Start, FRotator::ZeroRotator, FVector::OneVector,
		true, true, ENCPoolMethod::AutoRelease);

	if (NiagaraComp)
	{
		NiagaraComp->SetVariableVec3(FName("BeamEnd"), End);
	}
}
