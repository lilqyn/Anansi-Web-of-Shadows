// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/LevelTransition.h"
#include "AnansiGame.h"
#include "Player/AnansiCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ALevelTransition::ALevelTransition()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(200, 200, 200));
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBox;

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ALevelTransition::OnOverlap);
}

void ALevelTransition::OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bTriggered) return;
	if (!Cast<AAnansiCharacter>(OtherActor)) return;
	if (TargetLevelName.IsNone()) return;

	bTriggered = true;

	// Fade to black
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, FadeDuration, FLinearColor::Black, false, true);
	}

	// Load after fade
	FTimerHandle LoadTimer;
	GetWorldTimerManager().SetTimer(LoadTimer, this, &ALevelTransition::DoLevelLoad, FadeDuration, false);

	UE_LOG(LogAnansi, Log, TEXT("Level transition to: %s"), *TargetLevelName.ToString());
}

void ALevelTransition::DoLevelLoad()
{
	UGameplayStatics::OpenLevel(GetWorld(), TargetLevelName);
}
