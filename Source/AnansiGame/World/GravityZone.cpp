// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/GravityZone.h"
#include "AnansiGame.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AGravityZone::AGravityZone()
{
	PrimaryActorTick.bCanEverTick = false;

	GravityVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("GravityVolume"));
	GravityVolume->SetBoxExtent(FVector(300, 300, 300));
	GravityVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	GravityVolume->SetGenerateOverlapEvents(true);
	GravityVolume->SetHiddenInGame(false);
	GravityVolume->ShapeColor = FColor::Purple;
	RootComponent = GravityVolume;

	GravityVolume->OnComponentBeginOverlap.AddDynamic(this, &AGravityZone::OnOverlapBegin);
	GravityVolume->OnComponentEndOverlap.AddDynamic(this, &AGravityZone::OnOverlapEnd);
}

void AGravityZone::OnOverlapBegin(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* Char = Cast<ACharacter>(OtherActor);
	if (!Char || !Char->GetCharacterMovement()) return;

	Char->GetCharacterMovement()->GravityScale = GravityScale;
	UE_LOG(LogAnansi, Log, TEXT("%s entered gravity zone (scale: %.1f)"), *Char->GetName(), GravityScale);
}

void AGravityZone::OnOverlapEnd(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACharacter* Char = Cast<ACharacter>(OtherActor);
	if (!Char || !Char->GetCharacterMovement()) return;

	Char->GetCharacterMovement()->GravityScale = 1.0f;
	UE_LOG(LogAnansi, Log, TEXT("%s left gravity zone"), *Char->GetName());
}
