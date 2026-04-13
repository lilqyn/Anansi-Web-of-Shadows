// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "World/TrapPlate.h"
#include "AnansiGame.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

ATrapPlate::ATrapPlate()
{
	PrimaryActorTick.bCanEverTick = false;

	PlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
	RootComponent = PlateMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PlateMesh->SetStaticMesh(CubeMesh.Object);
		PlateMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.05f));
	}
	PlateMesh->SetCollisionProfileName(TEXT("BlockAll"));

	PlateTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("PlateTrigger"));
	PlateTrigger->SetupAttachment(RootComponent);
	PlateTrigger->SetBoxExtent(FVector(75, 75, 20));
	PlateTrigger->SetRelativeLocation(FVector(0, 0, 10));
	PlateTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	PlateTrigger->SetGenerateOverlapEvents(true);

	PlateTrigger->OnComponentBeginOverlap.AddDynamic(this, &ATrapPlate::OnSteppedOn);

	Tags.Add(FName("Trap"));
}

void ATrapPlate::OnSteppedOn(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsArmed) return;

	ACharacter* Victim = Cast<ACharacter>(OtherActor);
	if (!Victim) return;

	ActivateTrap(Victim);
}

void ATrapPlate::ActivateTrap(ACharacter* Victim)
{
	bIsArmed = false;

	switch (TrapType)
	{
	case ETrapType::SpikeLaunch:
		Victim->LaunchCharacter(FVector(0, 0, 1200), true, true);
		{
			FDamageEvent DmgEvent;
			Victim->TakeDamage(TrapDamage, DmgEvent, nullptr, this);
		}
		break;

	case ETrapType::SlowField:
		if (Victim->GetCharacterMovement())
		{
			Victim->GetCharacterMovement()->MaxWalkSpeed *= 0.3f;
			FTimerHandle SlowTimer;
			GetWorldTimerManager().SetTimer(SlowTimer, [Victim]()
			{
				if (Victim && Victim->GetCharacterMovement())
					Victim->GetCharacterMovement()->MaxWalkSpeed /= 0.3f;
			}, 3.0f, false);
		}
		break;

	case ETrapType::DamagePulse:
		{
			FDamageEvent DmgEvent;
			Victim->TakeDamage(TrapDamage, DmgEvent, nullptr, this);
		}
		break;
	}

	// Visual — plate sinks
	if (PlateMesh)
	{
		PlateMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.02f));
	}

	UE_LOG(LogAnansi, Log, TEXT("Trap activated on %s (type: %d)"), *Victim->GetName(), static_cast<int32>(TrapType));

	// Rearm after delay
	FTimerHandle RearmTimer;
	GetWorldTimerManager().SetTimer(RearmTimer, this, &ATrapPlate::RearmTrap, ResetTime, false);
}

void ATrapPlate::RearmTrap()
{
	bIsArmed = true;
	if (PlateMesh)
	{
		PlateMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.05f));
	}
}
