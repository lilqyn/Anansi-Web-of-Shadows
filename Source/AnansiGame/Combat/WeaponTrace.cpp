// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Combat/WeaponTrace.h"
#include "AnansiGame.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

UWeaponTraceComponent::UWeaponTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Disabled by default — we enable ticking only while tracing.
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UWeaponTraceComponent::StartTrace()
{
	bIsTracing = true;
	bHasPreviousPositions = false;
	HitActorsThisSwing.Reset();
	SetComponentTickEnabled(true);
}

void UWeaponTraceComponent::EndTrace()
{
	bIsTracing = false;
	bHasPreviousPositions = false;
	SetComponentTickEnabled(false);
}

void UWeaponTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsTracing)
	{
		return;
	}

	FVector CurrentStart, CurrentEnd;
	if (!GetSocketLocation(StartSocketName, CurrentStart) || !GetSocketLocation(EndSocketName, CurrentEnd))
	{
		return;
	}

	if (bHasPreviousPositions)
	{
		// Interpolate between previous and current positions to catch fast swings.
		for (int32 Step = 0; Step <= InterpolationSteps; ++Step)
		{
			const float Alpha = static_cast<float>(Step) / static_cast<float>(InterpolationSteps);
			const FVector InterpStart = FMath::Lerp(PrevStartLocation, CurrentStart, Alpha);
			const FVector InterpEnd = FMath::Lerp(PrevEndLocation, CurrentEnd, Alpha);
			PerformSweep(InterpStart, InterpEnd);
		}
	}
	else
	{
		// First frame of the trace — just do a single sweep.
		PerformSweep(CurrentStart, CurrentEnd);
	}

	PrevStartLocation = CurrentStart;
	PrevEndLocation = CurrentEnd;
	bHasPreviousPositions = true;
}

void UWeaponTraceComponent::PerformSweep(const FVector& Start, const FVector& End)
{
	if (!GetWorld())
	{
		return;
	}

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.bTraceComplex = false;
	Params.bReturnPhysicalMaterial = false;

	TArray<FHitResult> HitResults;
	const FCollisionShape Shape = FCollisionShape::MakeSphere(TraceRadius);

	const bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		Shape,
		Params
	);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(GetWorld(), Start, End, DebugColor, false, 0.1f, 0, TraceRadius * 0.25f);
	}
#endif

	if (!bHit)
	{
		return;
	}

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			continue;
		}

		// Skip actors we already hit during this swing.
		TWeakObjectPtr<AActor> WeakHit(HitActor);
		if (HitActorsThisSwing.Contains(WeakHit))
		{
			continue;
		}

		HitActorsThisSwing.Add(WeakHit);

		FWeaponHitResult Payload;
		Payload.HitActor = WeakHit;
		Payload.HitLocation = Hit.ImpactPoint;
		Payload.HitNormal = Hit.ImpactNormal;
		Payload.HitBoneName = Hit.BoneName;

		UE_LOG(LogAnansi, Verbose, TEXT("Weapon hit: %s at %s"), *HitActor->GetName(), *Hit.ImpactPoint.ToString());
		OnWeaponHit.Broadcast(Payload);
	}
}

bool UWeaponTraceComponent::GetSocketLocation(FName SocketName, FVector& OutLocation) const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// Look for a skeletal mesh component on the owner.
	const USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh)
	{
		return false;
	}

	if (!Mesh->DoesSocketExist(SocketName))
	{
		UE_LOG(LogAnansi, Warning, TEXT("WeaponTrace: Socket '%s' not found on mesh."), *SocketName.ToString());
		return false;
	}

	OutLocation = Mesh->GetSocketLocation(SocketName);
	return true;
}
