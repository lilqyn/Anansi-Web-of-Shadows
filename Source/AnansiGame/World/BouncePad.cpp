// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/BouncePad.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

ABouncePad::ABouncePad()
{
	PrimaryActorTick.bCanEverTick = true;

	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMesh"));
	RootComponent = PadMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PadMesh->SetStaticMesh(CubeMesh.Object);
		PadMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.15f));
	}
	PadMesh->SetCollisionProfileName(TEXT("BlockAll"));

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(100, 100, 30));
	TriggerBox->SetRelativeLocation(FVector(0, 0, 20));
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABouncePad::OnOverlap);
}

void ABouncePad::OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* Char = Cast<ACharacter>(OtherActor);
	if (!Char) return;

	if (CooldownTimer > 0.0f) return;
	CooldownTimer = Cooldown;

	Char->LaunchCharacter(LaunchVelocity, true, true);

	// Visual bounce — scale pulse on the pad
	if (PadMesh)
	{
		PadMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.08f));
		FTimerHandle ResetTimer;
		GetWorldTimerManager().SetTimer(ResetTimer, [this]()
		{
			if (PadMesh) PadMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.15f));
		}, 0.15f, false);
	}
}
