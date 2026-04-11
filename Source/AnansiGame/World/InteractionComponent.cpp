// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/InteractionComponent.h"
#include "World/AnansiInteractable.h"
#include "AnansiGame.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"

UAnansiInteractionComponent::UAnansiInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz is enough for interaction detection
}

void UAnansiInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	FindBestInteractable();
}

void UAnansiInteractionComponent::Interact()
{
	if (AAnansiInteractable* Target = FocusedInteractable.Get())
	{
		Target->OnInteract(Cast<ACharacter>(GetOwner()));
	}
}

void UAnansiInteractionComponent::FindBestInteractable()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector Origin = Owner->GetActorLocation();
	const FVector Forward = Owner->GetActorForwardVector();

	// Overlap sphere
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	GetWorld()->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_WorldDynamic, Sphere, Params);

	AAnansiInteractable* Best = nullptr;
	float BestScore = -1.0f;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AAnansiInteractable* Candidate = Cast<AAnansiInteractable>(Overlap.GetActor());
		if (!Candidate || !Candidate->CanInteract())
		{
			continue;
		}

		const FVector ToTarget = (Candidate->GetActorLocation() - Origin).GetSafeNormal();
		const float Dot = FVector::DotProduct(Forward, ToTarget);

		if (Dot < ViewDotThreshold)
		{
			continue;
		}

		// Score: higher dot product = more centered in view
		const float Distance = FVector::Dist(Origin, Candidate->GetActorLocation());
		const float Score = Dot * (1.0f - Distance / InteractionRadius);

		if (Score > BestScore)
		{
			BestScore = Score;
			Best = Candidate;
		}
	}

	// Update focused interactable
	if (Best != FocusedInteractable.Get())
	{
		if (FocusedInteractable.IsValid())
		{
			FocusedInteractable->OnLostFocus();
			OnInteractableLost.Broadcast();
		}

		FocusedInteractable = Best;

		if (Best)
		{
			Best->OnGainedFocus();
			OnInteractableFound.Broadcast(Best);
		}
	}
}
