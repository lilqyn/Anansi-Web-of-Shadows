// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Traversal/InteractionPoint.h"
#include "AnansiGame.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

AInteractionPoint::AInteractionPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// Detection sphere
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetSphereRadius(DetectionRadius);
	DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DetectionSphere->SetGenerateOverlapEvents(true);
	RootComponent = DetectionSphere;

	// Indicator mesh (hidden by default)
	IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndicatorMesh"));
	IndicatorMesh->SetupAttachment(RootComponent);
	IndicatorMesh->SetVisibility(false);
	IndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AInteractionPoint::BeginPlay()
{
	Super::BeginPlay();

	DetectionSphere->SetSphereRadius(DetectionRadius);
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AInteractionPoint::OnSphereBeginOverlap);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AInteractionPoint::OnSphereEndOverlap);

	Tags.Add(FName("Interactable"));
}

void AInteractionPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ---------------------------------------------------------------------------
// Interaction
// ---------------------------------------------------------------------------

bool AInteractionPoint::Interact(AActor* Interactor)
{
	if (!bIsAvailable || !Interactor)
	{
		return false;
	}

	if (!IsActorInRange(Interactor))
	{
		return false;
	}

	// Play montage on the interactor
	if (InteractionMontage)
	{
		if (ACharacter* Character = Cast<ACharacter>(Interactor))
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_Play(InteractionMontage);
			}
		}
	}

	OnActivated.Broadcast(this, Interactor);

	UE_LOG(LogAnansi, Log, TEXT("InteractionPoint [%s] activated by %s"),
		*GetName(), *Interactor->GetName());

	return true;
}

bool AInteractionPoint::IsActorInRange(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}
	return FVector::Dist(GetActorLocation(), Actor->GetActorLocation()) <= DetectionRadius;
}

void AInteractionPoint::SetAvailable(bool bAvailable)
{
	bIsAvailable = bAvailable;
	if (!bAvailable)
	{
		ShowIndicator(false);
	}
}

// ---------------------------------------------------------------------------
// Overlap callbacks
// ---------------------------------------------------------------------------

void AInteractionPoint::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !OtherActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}

	bPlayerInRange = true;
	if (bIsAvailable)
	{
		ShowIndicator(true);
	}
}

void AInteractionPoint::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || !OtherActor->ActorHasTag(TEXT("Player")))
	{
		return;
	}

	bPlayerInRange = false;
	ShowIndicator(false);
}

void AInteractionPoint::ShowIndicator(bool bShow)
{
	if (IndicatorMesh)
	{
		IndicatorMesh->SetVisibility(bShow);
	}
}
