// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/KeyPickup.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "Core/InventorySystem.h"
#include "UI/AnansiDevHUD.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AKeyPickup::AKeyPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectionSphere->SetSphereRadius(100.0f);
	CollectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollectionSphere->SetGenerateOverlapEvents(true);
	RootComponent = CollectionSphere;

	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	KeyMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		KeyMesh->SetStaticMesh(CubeMesh.Object);
		KeyMesh->SetRelativeScale3D(FVector(0.1f, 0.3f, 0.08f));
	}
	KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	KeyMesh->SetRenderCustomDepth(true);
	KeyMesh->SetCustomDepthStencilValue(3); // Gold

	CollectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AKeyPickup::OnOverlap);
}

void AKeyPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bCollected) return;

	if (SpawnLoc.IsZero()) SpawnLoc = GetActorLocation();

	BobTimer += DeltaTime;
	SetActorLocation(SpawnLoc + FVector(0, 0, FMath::Sin(BobTimer * 2.5f) * 8.0f));
	AddActorLocalRotation(FRotator(0, 80.0f * DeltaTime, 0));
}

void AKeyPickup::OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCollected || !Cast<AAnansiCharacter>(OtherActor)) return;

	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UInventorySystem* Inv = GI->GetSubsystem<UInventorySystem>())
		{
			Inv->AddItem(KeyID);
			bCollected = true;

			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
				{
					HUD->ShowToast(FString::Printf(TEXT("Picked up: %s"), *KeyName.ToString()),
						FColor(255, 200, 50));
				}
			}

			Destroy();
		}
	}
}
