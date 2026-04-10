// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Stealth/DetectionComponent.h"
#include "Stealth/StealthComponent.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UDetectionComponent::UDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz
}

void UDetectionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Spawn the awareness indicator widget above the owner
	if (AwarenessIndicatorWidgetClass && GetOwner())
	{
		IndicatorWidgetComp = NewObject<UWidgetComponent>(GetOwner(), TEXT("AwarenessIndicator"));
		if (IndicatorWidgetComp)
		{
			IndicatorWidgetComp->SetWidgetClass(AwarenessIndicatorWidgetClass);
			IndicatorWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
			IndicatorWidgetComp->SetDrawAtDesiredSize(true);
			IndicatorWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
			IndicatorWidgetComp->SetVisibility(false);
			IndicatorWidgetComp->RegisterComponent();
			IndicatorWidgetComp->AttachToComponent(
				GetOwner()->GetRootComponent(),
				FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
}

void UDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Find player
	const ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player)
	{
		return;
	}

	const UStealthComponent* Stealth = Player->FindComponentByClass<UStealthComponent>();

	// Evaluate detection
	const float VisualGain = EvaluateVisualDetection(Player, Stealth);
	const float AudioGain = EvaluateAudioDetection(Player, Stealth);
	const float TotalGain = (VisualGain + AudioGain) * DeltaTime;

	bHasStimulus = TotalGain > 0.0f;

	if (bHasStimulus)
	{
		Awareness = FMath::Min(Awareness + TotalGain, CombatThreshold);
		DecayTimer = DecayDelay;

		// Update last known position
		LastKnownPlayerPosition = Player->GetActorLocation();
		bHasLastKnownPosition = true;
	}
	else
	{
		// Decay awareness after delay
		if (DecayTimer > 0.0f)
		{
			DecayTimer -= DeltaTime;
		}
		else if (Awareness > 0.0f)
		{
			Awareness = FMath::Max(0.0f, Awareness - AwarenessDecayRate * DeltaTime);
		}
	}

	UpdateIndicator();
}

// ---------------------------------------------------------------------------
// Visual detection
// ---------------------------------------------------------------------------

float UDetectionComponent::EvaluateVisualDetection(const ACharacter* Player,
	const UStealthComponent* Stealth) const
{
	if (!Player || !GetOwner())
	{
		return 0.0f;
	}

	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	const FVector PlayerLocation = Player->GetActorLocation();
	const FVector OwnerForward = GetOwner()->GetActorForwardVector();

	// Distance check
	const float Distance = FVector::Dist(OwnerLocation, PlayerLocation);
	if (Distance > SightRange)
	{
		return 0.0f;
	}

	// Cone check
	const FVector ToPlayer = (PlayerLocation - OwnerLocation).GetSafeNormal();
	const float DotProduct = FVector::DotProduct(OwnerForward, ToPlayer);
	const float AngleCos = FMath::Cos(FMath::DegreesToRadians(SightHalfAngle));

	if (DotProduct < AngleCos)
	{
		return 0.0f;
	}

	// Line of sight check
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		Hit, OwnerLocation + FVector(0.0f, 0.0f, 60.0f),
		PlayerLocation + FVector(0.0f, 0.0f, 60.0f),
		ECC_Visibility, Params);

	if (bBlocked && Hit.GetActor() != Player)
	{
		return 0.0f;
	}

	// Distance falloff
	const float DistanceFactor = 1.0f - (Distance / SightRange);

	// Visibility from stealth component
	const float PlayerVisibility = Stealth ? Stealth->GetVisibilityScore() : 1.0f;

	return VisualAwarenessRate * DistanceFactor * PlayerVisibility;
}

// ---------------------------------------------------------------------------
// Audio detection
// ---------------------------------------------------------------------------

float UDetectionComponent::EvaluateAudioDetection(const ACharacter* Player,
	const UStealthComponent* Stealth) const
{
	if (!Player || !GetOwner())
	{
		return 0.0f;
	}

	const float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
	if (Distance > HearingRange)
	{
		return 0.0f;
	}

	const float PlayerNoise = Stealth ? Stealth->GetNoiseLevel() : 0.0f;
	if (PlayerNoise <= 0.0f)
	{
		return 0.0f;
	}

	const float DistanceFactor = 1.0f - (Distance / HearingRange);
	return AudioAwarenessRate * DistanceFactor * PlayerNoise;
}

// ---------------------------------------------------------------------------
// Awareness interface
// ---------------------------------------------------------------------------

void UDetectionComponent::AddAwareness(float Amount)
{
	Awareness = FMath::Clamp(Awareness + Amount, 0.0f, CombatThreshold);
	DecayTimer = DecayDelay;
}

void UDetectionComponent::SetFullAwareness()
{
	Awareness = CombatThreshold;
	DecayTimer = DecayDelay;
}

void UDetectionComponent::ResetAwareness()
{
	Awareness = 0.0f;
	bHasLastKnownPosition = false;
}

// ---------------------------------------------------------------------------
// Indicator
// ---------------------------------------------------------------------------

void UDetectionComponent::UpdateIndicator()
{
	if (!IndicatorWidgetComp)
	{
		return;
	}

	// Show indicator when suspicious or above
	const bool bShouldShow = Awareness >= SuspiciousThreshold;
	if (IndicatorWidgetComp->IsVisible() != bShouldShow)
	{
		IndicatorWidgetComp->SetVisibility(bShouldShow);
	}
}
