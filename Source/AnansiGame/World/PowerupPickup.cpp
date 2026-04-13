// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/PowerupPickup.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "Combat/MeleeDamageDealer.h"
#include "Combat/CombatComponent.h"
#include "UI/AnansiDevHUD.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

APowerupPickup::APowerupPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectionSphere->SetSphereRadius(80.0f);
	CollectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollectionSphere->SetGenerateOverlapEvents(true);
	RootComponent = CollectionSphere;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PickupMesh->SetStaticMesh(CubeMesh.Object);
		PickupMesh->SetRelativeScale3D(FVector(0.2f));
		PickupMesh->SetRelativeRotation(FRotator(45, 45, 0));
	}
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetRenderCustomDepth(true);
	PickupMesh->SetCustomDepthStencilValue(2);

	CollectionSphere->OnComponentBeginOverlap.AddDynamic(this, &APowerupPickup::OnOverlap);

	SetLifeSpan(30.0f);
}

void APowerupPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bCollected) return;

	if (SpawnLoc.IsZero()) SpawnLoc = GetActorLocation();

	BobTimer += DeltaTime;
	SetActorLocation(SpawnLoc + FVector(0, 0, FMath::Sin(BobTimer * 3.0f) * 12.0f));
	AddActorLocalRotation(FRotator(0, 90.0f * DeltaTime, 0));
}

void APowerupPickup::OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(OtherActor);
	if (!Anansi || bCollected) return;

	bCollected = true;

	FString PowerupName;

	switch (PowerupType)
	{
	case EPowerupType::SpeedBoost:
	{
		const float OldSpeed = Anansi->GetCharacterMovement()->MaxWalkSpeed;
		Anansi->GetCharacterMovement()->MaxWalkSpeed *= BoostMultiplier;
		PowerupName = TEXT("SPEED BOOST");

		FTimerHandle ResetTimer;
		GetWorldTimerManager().SetTimer(ResetTimer, [Anansi, OldSpeed]()
		{
			if (Anansi && Anansi->GetCharacterMovement())
				Anansi->GetCharacterMovement()->MaxWalkSpeed = OldSpeed;
		}, Duration, false);
		break;
	}

	case EPowerupType::DamageBoost:
		if (Anansi->MeleeDamage)
		{
			const float OldDmg = Anansi->MeleeDamage->BaseDamage;
			Anansi->MeleeDamage->BaseDamage *= BoostMultiplier;
			PowerupName = TEXT("DAMAGE BOOST");

			FTimerHandle ResetTimer;
			GetWorldTimerManager().SetTimer(ResetTimer, [Anansi, OldDmg]()
			{
				if (Anansi && Anansi->MeleeDamage)
					Anansi->MeleeDamage->BaseDamage = OldDmg;
			}, Duration, false);
		}
		break;

	case EPowerupType::Shield:
		if (Anansi->CombatComponent)
		{
			// Temporary invulnerability via combat component
			Anansi->CombatComponent->StartDodge(); // Grants i-frames
			PowerupName = TEXT("SHIELD");
		}
		break;
	}

	// HUD toast
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AAnansiDevHUD* HUD = Cast<AAnansiDevHUD>(PC->GetHUD()))
		{
			HUD->ShowToast(FString::Printf(TEXT("%s (%.0fs)"), *PowerupName, Duration), FColor(100, 255, 200));
		}
	}

	UE_LOG(LogAnansi, Log, TEXT("Powerup collected: %s (%.0fs, x%.1f)"), *PowerupName, Duration, BoostMultiplier);
	Destroy();
}
