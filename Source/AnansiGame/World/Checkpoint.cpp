// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/Checkpoint.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "UI/AnansiDevHUD.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

TWeakObjectPtr<ACheckpoint> ACheckpoint::ActiveCheckpoint;

ACheckpoint::ACheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetBoxExtent(FVector(150, 150, 200));
	TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerVolume->SetGenerateOverlapEvents(true);
	RootComponent = TriggerVolume;

	PillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh"));
	PillarMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylMesh.Succeeded())
	{
		PillarMesh->SetStaticMesh(CylMesh.Object);
		PillarMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 2.0f));
		PillarMesh->SetRelativeLocation(FVector(0, 0, -100));
	}
	PillarMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnOverlap);
}

void ACheckpoint::OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<AAnansiCharacter>(OtherActor)) return;
	if (bIsActivated) return;

	Activate();
}

void ACheckpoint::Activate()
{
	bIsActivated = true;
	ActiveCheckpoint = this;

	// Visual feedback — enable custom depth for glow effect
	if (PillarMesh)
	{
		PillarMesh->SetRenderCustomDepth(true);
		PillarMesh->SetCustomDepthStencilValue(2);
	}

	OnCheckpointActivated.Broadcast(this);

	// Show toast
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->ShowToast(FString::Printf(TEXT("Checkpoint: %s"), *CheckpointName.ToString()),
				FColor(100, 255, 100));
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("Checkpoint activated: %s"), *CheckpointName.ToString());
}

FVector ACheckpoint::GetRespawnLocation() const
{
	return GetActorLocation() + FVector(0, 0, 100);
}

ACheckpoint* ACheckpoint::GetActiveCheckpoint(UObject* WorldContext)
{
	return ActiveCheckpoint.Get();
}
