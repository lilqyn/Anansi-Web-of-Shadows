// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Stealth/CoverVolume.h"
#include "Stealth/StealthComponent.h"
#include "AnansiGame.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"

ACoverVolume::ACoverVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	CoverBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CoverBox"));
	CoverBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	CoverBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CoverBox->SetGenerateOverlapEvents(true);
	RootComponent = CoverBox;
}

void ACoverVolume::BeginPlay()
{
	Super::BeginPlay();

	CoverBox->OnComponentBeginOverlap.AddDynamic(this, &ACoverVolume::OnOverlapBegin);
	CoverBox->OnComponentEndOverlap.AddDynamic(this, &ACoverVolume::OnOverlapEnd);
}

void ACoverVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	UStealthComponent* Stealth = OtherActor->FindComponentByClass<UStealthComponent>();
	if (Stealth)
	{
		Stealth->SetCoverBonus(StealthBonus);
		UE_LOG(LogAnansi, Verbose, TEXT("Player entered %s cover (bonus %.2f)"),
			*UEnum::GetDisplayValueAsText(CoverType).ToString(), StealthBonus);
	}
}

void ACoverVolume::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	UStealthComponent* Stealth = OtherActor->FindComponentByClass<UStealthComponent>();
	if (Stealth)
	{
		Stealth->ClearCoverBonus();
		UE_LOG(LogAnansi, Verbose, TEXT("Player left cover"));
	}
}
