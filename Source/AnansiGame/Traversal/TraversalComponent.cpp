// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Traversal/TraversalComponent.h"
#include "Traversal/WebSwingPoint.h"
#include "AnansiGame.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"

UTraversalComponent::UTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTraversalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Tick cooldowns
	if (WallRunCooldownTimer > 0.0f)
	{
		WallRunCooldownTimer -= DeltaTime;
	}

	// Tick active traversal states
	switch (CurrentState)
	{
	case ETraversalState::WallRunning:
		TickWallRun(DeltaTime);
		break;
	case ETraversalState::WebSwinging:
		TickWebSwing(DeltaTime);
		break;
	default:
		break;
	}
}

// ---------------------------------------------------------------------------
// State machine
// ---------------------------------------------------------------------------

void UTraversalComponent::SetTraversalState(ETraversalState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const ETraversalState OldState = CurrentState;
	CurrentState = NewState;
	OnTraversalStateChanged.Broadcast(OldState, NewState);

	UE_LOG(LogAnansi, Log, TEXT("Traversal state: %d -> %d"),
		static_cast<int32>(OldState), static_cast<int32>(NewState));
}

// ---------------------------------------------------------------------------
// Wall running
// ---------------------------------------------------------------------------

bool UTraversalComponent::TryWallRun()
{
	if (CurrentState != ETraversalState::Grounded && CurrentState != ETraversalState::LedgeHanging)
	{
		return false;
	}

	if (WallRunCooldownTimer > 0.0f)
	{
		return false;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar || !OwnerChar->GetCharacterMovement()->IsFalling())
	{
		return false;
	}

	FHitResult WallHit;
	if (!DetectWall(WallHit))
	{
		return false;
	}

	WallNormal = WallHit.ImpactNormal;
	WallRunTimer = 0.0f;

	// Determine wall side
	const FVector Right = OwnerChar->GetActorRightVector();
	bWallRunRight = FVector::DotProduct(WallNormal, Right) < 0.0f;

	// Set movement mode
	OwnerChar->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	OwnerChar->GetCharacterMovement()->GravityScale = WallRunGravityScale;

	SetTraversalState(ETraversalState::WallRunning);
	return true;
}

void UTraversalComponent::EndWallRun()
{
	if (CurrentState != ETraversalState::WallRunning)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar)
	{
		OwnerChar->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		OwnerChar->GetCharacterMovement()->GravityScale = 1.0f;

		// Wall jump away
		const FVector JumpDirection = WallNormal * 400.0f + FVector(0.0f, 0.0f, 300.0f);
		OwnerChar->LaunchCharacter(JumpDirection, true, true);
	}

	WallRunCooldownTimer = WallRunCooldown;
	SetTraversalState(ETraversalState::Grounded);
}

bool UTraversalComponent::DetectWall(FHitResult& OutHit) const
{
	const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		return false;
	}

	const FVector Location = OwnerChar->GetActorLocation();
	const FVector Right = OwnerChar->GetActorRightVector();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerChar);

	// Check right
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Location,
		Location + Right * WallDetectionDistance, ECC_WorldStatic, Params))
	{
		return true;
	}

	// Check left
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Location,
		Location - Right * WallDetectionDistance, ECC_WorldStatic, Params))
	{
		return true;
	}

	return false;
}

void UTraversalComponent::TickWallRun(float DeltaTime)
{
	WallRunTimer += DeltaTime;

	if (WallRunTimer >= WallRunMaxDuration)
	{
		EndWallRun();
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		EndWallRun();
		return;
	}

	// Check wall is still there
	FHitResult WallHit;
	if (!DetectWall(WallHit))
	{
		EndWallRun();
		return;
	}

	WallNormal = WallHit.ImpactNormal;

	// Move along wall
	const FVector WallForward = FVector::CrossProduct(WallNormal, FVector::UpVector);
	const FVector MoveDirection = bWallRunRight ? WallForward : -WallForward;

	OwnerChar->GetCharacterMovement()->Velocity =
		MoveDirection * WallRunSpeed + FVector(0.0f, 0.0f, -50.0f * WallRunTimer);
}

// ---------------------------------------------------------------------------
// Web swinging
// ---------------------------------------------------------------------------

bool UTraversalComponent::TryWebSwing()
{
	if (CurrentState == ETraversalState::WebSwinging)
	{
		return false;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		return false;
	}

	AWebSwingPoint* SwingPoint = FindNearestSwingPoint();
	if (!SwingPoint)
	{
		return false;
	}

	ActiveSwingPoint = SwingPoint;
	SwingRopeLength = FVector::Dist(OwnerChar->GetActorLocation(), SwingPoint->GetActorLocation());

	// Initial angle from vertical
	const FVector ToPlayer = OwnerChar->GetActorLocation() - SwingPoint->GetActorLocation();
	SwingAngle = FMath::Atan2(ToPlayer.X, -ToPlayer.Z);
	SwingAngularVelocity = 0.0f;

	// Disable normal movement
	OwnerChar->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	OwnerChar->GetCharacterMovement()->GravityScale = 0.0f;

	SetTraversalState(ETraversalState::WebSwinging);

	UE_LOG(LogAnansi, Log, TEXT("Web swing attached (rope length %.0f)"), SwingRopeLength);
	return true;
}

void UTraversalComponent::ReleaseWebSwing()
{
	if (CurrentState != ETraversalState::WebSwinging)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar)
	{
		// Launch with preserved momentum
		const FVector ReleaseVelocity = CalculateSwingVelocity();
		OwnerChar->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		OwnerChar->GetCharacterMovement()->GravityScale = 1.0f;
		OwnerChar->LaunchCharacter(ReleaseVelocity, true, true);

		UE_LOG(LogAnansi, Log, TEXT("Web swing released (velocity %.0f)"), ReleaseVelocity.Size());
	}

	ActiveSwingPoint = nullptr;
	SetTraversalState(ETraversalState::Grounded);
}

void UTraversalComponent::ApplySwingInput(float ForwardInput, float RightInput)
{
	if (CurrentState != ETraversalState::WebSwinging)
	{
		return;
	}

	// Apply torque to the pendulum
	SwingAngularVelocity += ForwardInput * SwingInputForce * GetWorld()->GetDeltaSeconds() / SwingRopeLength;
}

AWebSwingPoint* UTraversalComponent::FindNearestSwingPoint() const
{
	const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		return nullptr;
	}

	const FVector Location = OwnerChar->GetActorLocation();
	AWebSwingPoint* Nearest = nullptr;
	float NearestDist = SwingPointDetectionRadius;

	for (TActorIterator<AWebSwingPoint> It(GetWorld()); It; ++It)
	{
		AWebSwingPoint* Point = *It;
		if (!Point)
		{
			continue;
		}

		// Only attach to points above the player
		if (Point->GetActorLocation().Z <= Location.Z)
		{
			continue;
		}

		const float Dist = FVector::Dist(Location, Point->GetActorLocation());
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			Nearest = Point;
		}
	}

	return Nearest;
}

void UTraversalComponent::TickWebSwing(float DeltaTime)
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar || !ActiveSwingPoint)
	{
		ReleaseWebSwing();
		return;
	}

	// Pendulum physics
	const float GravityAccel = SwingGravity / SwingRopeLength;
	SwingAngularVelocity -= GravityAccel * FMath::Sin(SwingAngle) * DeltaTime;
	SwingAngularVelocity *= SwingDamping;
	SwingAngle += SwingAngularVelocity * DeltaTime;

	// Calculate new position
	const FVector PivotLocation = ActiveSwingPoint->GetActorLocation();
	const FVector NewLocation = PivotLocation + FVector(
		FMath::Sin(SwingAngle) * SwingRopeLength,
		0.0f,
		-FMath::Cos(SwingAngle) * SwingRopeLength
	);

	OwnerChar->SetActorLocation(NewLocation, true);
}

FVector UTraversalComponent::CalculateSwingVelocity() const
{
	// Tangential velocity from pendulum motion
	const float TangentialSpeed = SwingAngularVelocity * SwingRopeLength;
	const FVector TangentDirection(FMath::Cos(SwingAngle), 0.0f, FMath::Sin(SwingAngle));
	return TangentDirection * TangentialSpeed;
}

// ---------------------------------------------------------------------------
// Ledge / mantle
// ---------------------------------------------------------------------------

bool UTraversalComponent::TryLedgeGrab()
{
	FVector LedgeLoc, LedgeNorm;
	if (!DetectLedge(LedgeLoc, LedgeNorm))
	{
		return false;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		return false;
	}

	LedgeLocation = LedgeLoc;
	LedgeNormal = LedgeNorm;

	// Hang from ledge
	OwnerChar->GetCharacterMovement()->DisableMovement();
	OwnerChar->SetActorLocation(LedgeLoc + LedgeNorm * 40.0f + FVector(0.0f, 0.0f, -80.0f));

	SetTraversalState(ETraversalState::LedgeHanging);
	return true;
}

void UTraversalComponent::ClimbUpFromLedge()
{
	if (CurrentState != ETraversalState::LedgeHanging)
	{
		return;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		return;
	}

	// Teleport to ledge top
	OwnerChar->SetActorLocation(LedgeLocation + FVector(0.0f, 0.0f, 10.0f));
	OwnerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	SetTraversalState(ETraversalState::Grounded);
}

bool UTraversalComponent::TryMantle()
{
	if (CurrentState != ETraversalState::Grounded)
	{
		return false;
	}

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		return false;
	}

	const FVector Location = OwnerChar->GetActorLocation();
	const FVector Forward = OwnerChar->GetActorForwardVector();

	// Forward trace to find obstacle
	FHitResult ForwardHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerChar);

	if (!GetWorld()->LineTraceSingleByChannel(ForwardHit, Location + FVector(0.0f, 0.0f, 50.0f),
		Location + Forward * LedgeDetectionDistance + FVector(0.0f, 0.0f, 50.0f),
		ECC_WorldStatic, Params))
	{
		return false;
	}

	// Height trace to find top of obstacle
	FHitResult HeightHit;
	const FVector HeightTraceStart = ForwardHit.ImpactPoint + Forward * 10.0f + FVector(0.0f, 0.0f, MantleMaxHeight + 50.0f);
	const FVector HeightTraceEnd = ForwardHit.ImpactPoint + Forward * 10.0f;

	if (!GetWorld()->LineTraceSingleByChannel(HeightHit, HeightTraceStart, HeightTraceEnd, ECC_WorldStatic, Params))
	{
		return false;
	}

	const float ObstacleHeight = HeightHit.ImpactPoint.Z - Location.Z;
	if (ObstacleHeight > MantleMaxHeight || ObstacleHeight < 20.0f)
	{
		return false;
	}

	// Play mantle montage
	if (MantleMontage && OwnerChar->GetMesh() && OwnerChar->GetMesh()->GetAnimInstance())
	{
		OwnerChar->GetMesh()->GetAnimInstance()->Montage_Play(MantleMontage);
	}

	// Move to top of obstacle
	const FVector TargetLocation = HeightHit.ImpactPoint + FVector::UpVector * OwnerChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	OwnerChar->SetActorLocation(TargetLocation, true);

	SetTraversalState(ETraversalState::Mantling);

	// Brief delay then return to grounded
	FTimerHandle MantleTimer;
	GetWorld()->GetTimerManager().SetTimer(MantleTimer, [this]()
	{
		SetTraversalState(ETraversalState::Grounded);
	}, 0.5f, false);

	return true;
}

bool UTraversalComponent::DetectLedge(FVector& OutLedgeLocation, FVector& OutLedgeNormal) const
{
	const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar)
	{
		return false;
	}

	const FVector Location = OwnerChar->GetActorLocation();
	const FVector Forward = OwnerChar->GetActorForwardVector();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerChar);

	// Forward trace at head height
	FHitResult ForwardHit;
	const FVector HeadLevel = Location + FVector(0.0f, 0.0f, LedgeGrabHeight);
	if (!GetWorld()->LineTraceSingleByChannel(ForwardHit, HeadLevel,
		HeadLevel + Forward * LedgeDetectionDistance, ECC_WorldStatic, Params))
	{
		return false;
	}

	// Downward trace to find the ledge top
	FHitResult DownHit;
	const FVector DownTraceStart = ForwardHit.ImpactPoint + Forward * 5.0f + FVector(0.0f, 0.0f, 50.0f);
	const FVector DownTraceEnd = ForwardHit.ImpactPoint + Forward * 5.0f - FVector(0.0f, 0.0f, 100.0f);

	if (!GetWorld()->LineTraceSingleByChannel(DownHit, DownTraceStart, DownTraceEnd, ECC_WorldStatic, Params))
	{
		return false;
	}

	// Verify the surface is mostly horizontal
	if (DownHit.ImpactNormal.Z < 0.7f)
	{
		return false;
	}

	OutLedgeLocation = DownHit.ImpactPoint;
	OutLedgeNormal = ForwardHit.ImpactNormal;
	return true;
}
