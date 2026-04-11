// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/AnansiInteractable.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"

AAnansiInteractable::AAnansiInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void AAnansiInteractable::OnInteract(ACharacter* InteractingCharacter)
{
	if (!CanInteract())
	{
		return;
	}

	OnInteracted.Broadcast(InteractingCharacter);

	UE_LOG(LogAnansi, Log, TEXT("Interacted with %s (type: %d)"),
		*GetName(), static_cast<int32>(InteractableType));

	if (bSingleUse)
	{
		bHasBeenUsed = true;
		bIsEnabled = false;
	}
}

bool AAnansiInteractable::CanInteract() const
{
	return bIsEnabled && !bHasBeenUsed;
}

void AAnansiInteractable::OnGainedFocus()
{
	UE_LOG(LogAnansi, Verbose, TEXT("%s gained focus"), *GetName());
}

void AAnansiInteractable::OnLostFocus()
{
	UE_LOG(LogAnansi, Verbose, TEXT("%s lost focus"), *GetName());
}
