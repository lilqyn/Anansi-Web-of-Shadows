// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/LootChest.h"
#include "World/LootDrop.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "Core/AnansiSaveManager.h"
#include "Core/InventorySystem.h"
#include "UI/AnansiDevHUD.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Character.h"

ALootChest::ALootChest()
{
	InteractableType = EInteractableType::Pickup;
	InteractionPrompt = NSLOCTEXT("Anansi", "OpenChest", "Open Chest");

	ChestBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestBase"));
	ChestBase->SetupAttachment(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		ChestBase->SetStaticMesh(CubeMesh.Object);
		ChestBase->SetRelativeScale3D(FVector(0.8f, 1.2f, 0.5f));
	}

	ChestLid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestLid"));
	ChestLid->SetupAttachment(ChestBase);
	if (CubeMesh.Succeeded())
	{
		ChestLid->SetStaticMesh(CubeMesh.Object);
		ChestLid->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.3f));
		ChestLid->SetRelativeLocation(FVector(0, 0, 65));
	}

	// Spider sense highlight
	ChestBase->SetRenderCustomDepth(true);
	ChestBase->SetCustomDepthStencilValue(3); // Gold
}

bool ALootChest::CanInteract() const
{
	return !bIsOpen && !bHasBeenUsed;
}

void ALootChest::OnInteract(ACharacter* InteractingCharacter)
{
	if (bIsOpen) return;

	if (bLocked)
	{
		// Check if player has the required key
		bool bHasKey = false;
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			if (UInventorySystem* Inv = GI->GetSubsystem<UInventorySystem>())
			{
				if (Inv->HasItem(RequiredKeyID))
				{
					Inv->RemoveItem(RequiredKeyID);
					bHasKey = true;
					bLocked = false;
				}
			}
		}

		if (!bHasKey)
		{
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
				{
					HUD->ShowToast(FString::Printf(TEXT("Locked! Requires: %s"), *RequiredKeyID.ToString()),
						FColor(255, 150, 50));
				}
			}
			return;
		}
	}

	bIsOpen = true;
	Super::OnInteract(InteractingCharacter);

	// Open the lid (rotate 60 degrees back)
	if (ChestLid)
	{
		ChestLid->SetRelativeRotation(FRotator(60.0f, 0, 0));
	}

	// Spawn loot orbs in a small fountain
	for (int32 i = 0; i < LootCount; ++i)
	{
		const FVector Offset(
			FMath::FRandRange(-100.0f, 100.0f),
			FMath::FRandRange(-100.0f, 100.0f),
			100.0f + FMath::FRandRange(0.0f, 50.0f));
		ALootDrop::SpawnRandomLoot(this, GetActorLocation() + Offset);
	}

	// Toast
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->ShowToast(TEXT("Chest opened!"), FColor(100, 255, 150));
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("Chest opened — %d loot drops"), LootCount);
}
