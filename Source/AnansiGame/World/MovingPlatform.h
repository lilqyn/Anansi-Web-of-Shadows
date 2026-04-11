// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatform.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EPlatformMotion : uint8
{
	Linear		UMETA(DisplayName = "Linear (A to B)"),
	Circular	UMETA(DisplayName = "Circular"),
	Pendulum	UMETA(DisplayName = "Pendulum (Swing)")
};

/**
 * AMovingPlatform
 *
 * Platform that moves between points, supporting linear, circular,
 * and pendulum motion. Characters standing on it ride along.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API AMovingPlatform : public AActor
{
	GENERATED_BODY()

public:
	AMovingPlatform();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Platform")
	EPlatformMotion MotionType = EPlatformMotion::Linear;

	/** End point offset from start (for Linear mode). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Platform")
	FVector EndOffset = FVector(0, 0, 500);

	/** Speed of movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Platform", meta = (ClampMin = "10.0"))
	float Speed = 200.0f;

	/** Pause time at each end of the path (Linear mode). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Platform", meta = (ClampMin = "0.0"))
	float PauseTime = 1.0f;

	/** Radius for circular/pendulum motion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Platform", meta = (ClampMin = "50.0"))
	float Radius = 300.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

private:
	FVector StartLocation;
	float MotionTimer = 0.0f;
	float PauseTimer = 0.0f;
	bool bMovingForward = true;
};
