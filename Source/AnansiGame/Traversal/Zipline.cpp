// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Traversal/Zipline.h"
#include "AnansiGame.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

AZipline::AZipline()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	StartTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("StartTrigger"));
	StartTrigger->SetupAttachment(RootComponent);
	StartTrigger->SetSphereRadius(100.0f);
	StartTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	StartTrigger->SetGenerateOverlapEvents(true);

	StartTrigger->OnComponentBeginOverlap.AddDynamic(this, &AZipline::OnStartOverlap);

	Tags.Add(FName("Zipline"));
}

void AZipline::OnStartOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (RidingCharacter) return;

	ACharacter* Char = Cast<ACharacter>(OtherActor);
	if (!Char) return;

	RidingCharacter = Char;
	RideProgress = 0.0f;
	StartWorldLocation = GetActorLocation();
	EndWorldLocation = GetActorLocation() + EndOffset;

	// Disable gravity and movement while riding
	if (Char->GetCharacterMovement())
	{
		Char->GetCharacterMovement()->GravityScale = 0.0f;
		Char->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}

	UE_LOG(LogAnansi, Log, TEXT("Zipline: Started riding"));
}

void AZipline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Draw the cable line
	if (StartWorldLocation.IsZero())
	{
		StartWorldLocation = GetActorLocation();
		EndWorldLocation = GetActorLocation() + EndOffset;
	}

#if ENABLE_DRAW_DEBUG
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + EndOffset,
		FColor::Silver, false, -1.0f, 0, 2.0f);
#endif

	if (!RidingCharacter) return;

	// Check if rider jumped off (velocity indicates jump input)
	if (RidingCharacter->bPressedJump)
	{
		if (RidingCharacter->GetCharacterMovement())
		{
			RidingCharacter->GetCharacterMovement()->GravityScale = 1.0f;
		}
		RidingCharacter->LaunchCharacter(FVector(0, 0, 400), false, true);
		RidingCharacter = nullptr;
		UE_LOG(LogAnansi, Log, TEXT("Zipline: Rider jumped off"));
		return;
	}

	// Advance along line
	const float TotalDist = FVector::Dist(StartWorldLocation, EndWorldLocation);
	RideProgress += (ZipSpeed * DeltaTime) / TotalDist;

	if (RideProgress >= 1.0f)
	{
		// Reached the end
		RidingCharacter->SetActorLocation(EndWorldLocation);
		if (RidingCharacter->GetCharacterMovement())
		{
			RidingCharacter->GetCharacterMovement()->GravityScale = 1.0f;
		}
		RidingCharacter = nullptr;
		UE_LOG(LogAnansi, Log, TEXT("Zipline: Ride complete"));
		return;
	}

	// Move rider along the line
	const FVector NewLoc = FMath::Lerp(StartWorldLocation, EndWorldLocation, RideProgress);
	RidingCharacter->SetActorLocation(NewLoc);
}
