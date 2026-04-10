// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Abilities/GA_SpiderSense.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Materials/MaterialInstanceDynamic.h"

UGA_SpiderSense::UGA_SpiderSense()
{
	EnergyCost = 25.0f;
	CooldownDuration = 12.0f;
}

void UGA_SpiderSense::OnAbilityActivated(const FGameplayAbilitySpecHandle Handle,
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

	UWorld* World = OwnerChar->GetWorld();

	// Apply slow motion
	UGameplayStatics::SetGlobalTimeDilation(World, SlowMotionTimeDilation);

	// Schedule time-dilation restore (uses real-time timer)
	FTimerDelegate SlowMoDelegate;
	SlowMoDelegate.BindUObject(this, &UGA_SpiderSense::RestoreTimeDilation);
	World->GetTimerManager().SetTimer(SlowMoTimerHandle, SlowMoDelegate, SlowMotionDuration, false);

	// Perform detection scan
	PerformDetectionScan(OwnerChar->GetActorLocation(), World);

	// Schedule ability end
	FTimerDelegate SenseDelegate;
	SenseDelegate.BindUObject(this, &UGA_SpiderSense::OnSenseExpired);
	World->GetTimerManager().SetTimer(SenseTimerHandle, SenseDelegate, SenseDuration, false);

	UE_LOG(LogAnansi, Log, TEXT("SpiderSense — activated (radius %.0f, duration %.1fs)"),
		DetectionRadius, SenseDuration);
}

void UGA_SpiderSense::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearHighlights();
	RestoreTimeDilation();

	if (const ACharacter* OwnerChar = GetOwnerCharacter())
	{
		UWorld* World = OwnerChar->GetWorld();
		World->GetTimerManager().ClearTimer(SlowMoTimerHandle);
		World->GetTimerManager().ClearTimer(SenseTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------

void UGA_SpiderSense::PerformDetectionScan(const FVector& Origin, UWorld* World)
{
	if (!World)
	{
		return;
	}

	// Overlap sphere to find highlightable actors
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(DetectionRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwnerCharacter());

	World->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_Visibility, Sphere, Params);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor || HighlightedActors.Contains(Actor))
		{
			continue;
		}

		// Enemies get stencil value 1 (red in typical setup)
		if (Actor->ActorHasTag(TEXT("Enemy")))
		{
			HighlightActor(Actor, 1);
			continue;
		}

		// Interactables get stencil value 2 (blue/teal)
		if (Actor->ActorHasTag(InteractableTag))
		{
			HighlightActor(Actor, 2);
			continue;
		}

		// Hidden collectibles get stencil value 3 (gold)
		if (Actor->ActorHasTag(CollectibleTag))
		{
			HighlightActor(Actor, 3);
			continue;
		}

		// Traps get stencil value 4 (orange/warning)
		if (Actor->ActorHasTag(TrapTag))
		{
			HighlightActor(Actor, 4);
			continue;
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("SpiderSense — detected %d actors"), HighlightedActors.Num());
}

void UGA_SpiderSense::HighlightActor(AActor* Actor, int32 StencilValue)
{
	if (!Actor)
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* Comp : PrimitiveComponents)
	{
		if (Comp && Comp->IsVisible())
		{
			Comp->SetRenderCustomDepth(true);
			Comp->SetCustomDepthStencilValue(StencilValue);
		}
	}

	HighlightedActors.Add(Actor);
}

void UGA_SpiderSense::ClearHighlights()
{
	for (AActor* Actor : HighlightedActors)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

		for (UPrimitiveComponent* Comp : PrimitiveComponents)
		{
			if (Comp)
			{
				Comp->SetRenderCustomDepth(false);
				Comp->SetCustomDepthStencilValue(0);
			}
		}
	}

	HighlightedActors.Empty();
}

void UGA_SpiderSense::RestoreTimeDilation()
{
	if (const ACharacter* OwnerChar = GetOwnerCharacter())
	{
		UGameplayStatics::SetGlobalTimeDilation(OwnerChar->GetWorld(), 1.0f);
	}
}

void UGA_SpiderSense::OnSenseExpired()
{
	if (IsActive())
	{
		K2_EndAbility();
	}
}
