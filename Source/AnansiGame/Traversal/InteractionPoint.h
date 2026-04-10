// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionPoint.generated.h"

class USphereComponent;
class UWidgetComponent;

/**
 * Interaction point type — determines which traversal or interaction behavior is triggered.
 */
UENUM(BlueprintType)
enum class EInteractionPointType : uint8
{
	SwingPoint		UMETA(DisplayName = "Swing Point"),
	ClimbSurface	UMETA(DisplayName = "Climb Surface"),
	ZipLine			UMETA(DisplayName = "Zip Line"),
	SpiritThread	UMETA(DisplayName = "Spirit Thread")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionPointActivated, AInteractionPoint*, Point, AActor*, Interactor);

/**
 * AInteractionPoint
 *
 * Base class for traversal interaction points in the world. Provides a
 * detection radius with an in-range visual indicator and fires a delegate
 * when the player interacts.
 */
UCLASS()
class ANANSIGAME_API AInteractionPoint : public AActor
{
	GENERATED_BODY()

public:
	AInteractionPoint();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// -------------------------------------------------------------------
	// Interaction interface
	// -------------------------------------------------------------------

	/** Attempt interaction from the given actor. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Interaction")
	virtual bool Interact(AActor* Interactor);

	/** Is the given actor within detection range? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Interaction")
	bool IsActorInRange(AActor* Actor) const;

	/** Is this point currently available for interaction? */
	UFUNCTION(BlueprintPure, Category = "Anansi|Interaction")
	bool IsAvailable() const { return bIsAvailable; }

	UFUNCTION(BlueprintCallable, Category = "Anansi|Interaction")
	void SetAvailable(bool bAvailable);

	// -------------------------------------------------------------------
	// Properties
	// -------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Anansi|Interaction")
	EInteractionPointType GetPointType() const { return PointType; }

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Interaction")
	FOnInteractionPointActivated OnActivated;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anansi|Interaction")
	EInteractionPointType PointType = EInteractionPointType::SwingPoint;

	/** Radius for auto-detection when the player is nearby. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anansi|Interaction", meta = (ClampMin = "50.0"))
	float DetectionRadius = 300.0f;

	/** Animation montage to play on the interactor when this point is used. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anansi|Interaction")
	TObjectPtr<UAnimMontage> InteractionMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Interaction")
	TObjectPtr<USphereComponent> DetectionSphere;

	/** Mesh or sprite for the visual indicator. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anansi|Interaction")
	TObjectPtr<UStaticMeshComponent> IndicatorMesh;

private:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ShowIndicator(bool bShow);

	bool bIsAvailable = true;
	bool bPlayerInRange = false;
};
