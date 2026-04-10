// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "AI/PatrolComponent.h"
#include "AnansiGame.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"

UPatrolComponent::UPatrolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPatrolComponent::BeginPlay()
{
	Super::BeginPlay();
	ResolveSplinePoints();
}

// ---------------------------------------------------------------------------
// Patrol interface
// ---------------------------------------------------------------------------

FVector UPatrolComponent::GetCurrentPatrolPoint() const
{
	const TArray<FVector>& Points = ResolvedPoints.Num() > 0 ? ResolvedPoints : PatrolPoints;
	if (Points.Num() == 0)
	{
		return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	return ApplyDeviation(Points[CurrentPointIndex]);
}

FVector UPatrolComponent::AdvanceToNextPoint()
{
	const TArray<FVector>& Points = ResolvedPoints.Num() > 0 ? ResolvedPoints : PatrolPoints;
	if (Points.Num() == 0)
	{
		return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	CurrentPointIndex = (CurrentPointIndex + 1) % Points.Num();
	return GetCurrentPatrolPoint();
}

float UPatrolComponent::GetCurrentWaitTime() const
{
	if (WaitTimes.IsValidIndex(CurrentPointIndex))
	{
		return WaitTimes[CurrentPointIndex];
	}
	return DefaultWaitTime;
}

int32 UPatrolComponent::GetPatrolPointCount() const
{
	const TArray<FVector>& Points = ResolvedPoints.Num() > 0 ? ResolvedPoints : PatrolPoints;
	return Points.Num();
}

bool UPatrolComponent::HasPatrolPath() const
{
	return GetPatrolPointCount() > 0;
}

void UPatrolComponent::WritePatrolTargetToBlackboard(UBlackboardComponent* Blackboard, FName KeyName)
{
	if (Blackboard)
	{
		Blackboard->SetValueAsVector(KeyName, GetCurrentPatrolPoint());
	}
}

// ---------------------------------------------------------------------------
// Internal
// ---------------------------------------------------------------------------

void UPatrolComponent::ResolveSplinePoints()
{
	ResolvedPoints.Empty();

	if (!SplineActor)
	{
		return;
	}

	const USplineComponent* Spline = SplineActor->FindComponentByClass<USplineComponent>();
	if (!Spline)
	{
		UE_LOG(LogAnansi, Warning, TEXT("PatrolComponent — SplineActor has no USplineComponent"));
		return;
	}

	const int32 NumPoints = Spline->GetNumberOfSplinePoints();
	ResolvedPoints.Reserve(NumPoints);

	for (int32 i = 0; i < NumPoints; ++i)
	{
		ResolvedPoints.Add(Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
	}

	UE_LOG(LogAnansi, Log, TEXT("PatrolComponent — resolved %d spline points"), NumPoints);
}

FVector UPatrolComponent::ApplyDeviation(const FVector& Point) const
{
	if (RandomDeviation <= 0.0f)
	{
		return Point;
	}

	const float DeviationX = FMath::RandRange(-RandomDeviation, RandomDeviation);
	const float DeviationY = FMath::RandRange(-RandomDeviation, RandomDeviation);
	return Point + FVector(DeviationX, DeviationY, 0.0f);
}
