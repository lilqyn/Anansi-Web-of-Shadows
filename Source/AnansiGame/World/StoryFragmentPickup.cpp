// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/StoryFragmentPickup.h"
#include "AnansiGame.h"
#include "Narrative/StoryFragmentSystem.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "UI/AnansiDevHUD.h"
#include "Core/QuestSystem.h"

AStoryFragmentPickup::AStoryFragmentPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	InteractableType = EInteractableType::StoryFragment;
	InteractionPrompt = NSLOCTEXT("Anansi", "PickupFragment", "Collect Fragment");
	bSingleUse = true;

	// Golden sphere visual
	FragmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FragmentMesh"));
	FragmentMesh->SetupAttachment(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		FragmentMesh->SetStaticMesh(SphereMesh.Object);
		FragmentMesh->SetRelativeScale3D(FVector(0.25f));
	}

	FragmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FragmentMesh->SetRenderCustomDepth(true);
	FragmentMesh->SetCustomDepthStencilValue(3); // Gold stencil for Spider Sense

	Tags.Add(FName("Collectible"));
}

void AStoryFragmentPickup::OnInteract(ACharacter* InteractingCharacter)
{
	Super::OnInteract(InteractingCharacter);

	// Add to the story fragment system
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UStoryFragmentSystem* FragSystem = GI->GetSubsystem<UStoryFragmentSystem>())
		{
			FragSystem->CollectFragment(FragmentID);
		}

		// Advance fragment-based quest objectives
		if (UQuestSystem* Quest = GI->GetSubsystem<UQuestSystem>())
		{
			if (Quest->HasActiveObjective() &&
				Quest->GetCurrentObjective().Type == EObjectiveType::CollectFragments)
			{
				Quest->ProgressObjective(1);
			}
		}
	}

	// Show toast
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->ShowToast(FString::Printf(TEXT("Fragment Collected: %s"), *FragmentTitle.ToString()),
				FColor(255, 200, 50));
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("Collected story fragment: %s — %s"),
		*FragmentID.ToString(), *FragmentTitle.ToString());

	// Disappear
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetLifeSpan(1.0f);
}

void AStoryFragmentPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHasBeenUsed) return;

	// Cache initial location on first tick
	if (InitialLocation.IsZero())
	{
		InitialLocation = GetActorLocation();
	}

	// Rotate
	AddActorLocalRotation(FRotator(0, RotationSpeed * DeltaTime, 0));

	// Bob up and down
	BobTimer += DeltaTime;
	const float BobOffset = FMath::Sin(BobTimer * 2.0f) * BobAmplitude;
	SetActorLocation(InitialLocation + FVector(0, 0, BobOffset));
}
