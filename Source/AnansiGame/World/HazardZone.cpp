// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/HazardZone.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AHazardZone::AHazardZone()
{
	PrimaryActorTick.bCanEverTick = true;

	HazardVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("HazardVolume"));
	HazardVolume->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
	HazardVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	HazardVolume->SetGenerateOverlapEvents(true);
	RootComponent = HazardVolume;

	HazardVolume->OnComponentBeginOverlap.AddDynamic(this, &AHazardZone::OnOverlapBegin);
	HazardVolume->OnComponentEndOverlap.AddDynamic(this, &AHazardZone::OnOverlapEnd);
}

void AHazardZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OverlappingActors.Num() == 0) return;

	DamageTickTimer += DeltaTime;
	if (DamageTickTimer < DamageTickInterval) return;

	DamageTickTimer = 0.0f;
	const float Damage = DamagePerSecond * DamageTickInterval;

	for (auto It = OverlappingActors.CreateIterator(); It; ++It)
	{
		AActor* Actor = It->Get();
		if (!Actor)
		{
			It.RemoveCurrent();
			continue;
		}

		FDamageEvent DamageEvent;
		Actor->TakeDamage(Damage, DamageEvent, nullptr, this);

		// Spirit hazard drains web energy
		if (HazardType == EHazardType::Spirit)
		{
			if (AAnansiCharacter* Anansi = Cast<AAnansiCharacter>(Actor))
			{
				Anansi->ConsumeWebEnergy(Damage * 0.5f);
			}
		}
	}
}

void AHazardZone::OnOverlapBegin(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* Char = Cast<ACharacter>(OtherActor);
	if (!Char) return;

	OverlappingActors.Add(OtherActor);

	// Poison slows
	if (HazardType == EHazardType::Poison && Char->GetCharacterMovement())
	{
		Char->GetCharacterMovement()->MaxWalkSpeed *= PoisonSlowMultiplier;
	}

	UE_LOG(LogAnansi, Log, TEXT("%s entered %s hazard zone"), *OtherActor->GetName(),
		HazardType == EHazardType::Fire ? TEXT("Fire") :
		HazardType == EHazardType::Poison ? TEXT("Poison") : TEXT("Spirit"));
}

void AHazardZone::OnOverlapEnd(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	OverlappingActors.Remove(OtherActor);

	// Restore speed when leaving poison
	if (HazardType == EHazardType::Poison)
	{
		if (ACharacter* Char = Cast<ACharacter>(OtherActor))
		{
			if (Char->GetCharacterMovement())
			{
				Char->GetCharacterMovement()->MaxWalkSpeed /= PoisonSlowMultiplier;
			}
		}
	}
}
