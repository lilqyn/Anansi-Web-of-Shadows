// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Checkpoint.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCheckpointActivated, ACheckpoint*, Checkpoint);

/**
 * ACheckpoint
 *
 * Save/respawn point. When the player enters the volume, this becomes
 * the active checkpoint. On death, the player respawns here instead
 * of the level's PlayerStart.
 *
 * Visual: glowing pillar that lights up when activated.
 */
UCLASS(Blueprintable)
class ANANSIGAME_API ACheckpoint : public AActor
{
	GENERATED_BODY()

public:
	ACheckpoint();

	UFUNCTION(BlueprintPure, Category = "Anansi|Checkpoint")
	bool IsActivated() const { return bIsActivated; }

	/** Manually activate this checkpoint. */
	UFUNCTION(BlueprintCallable, Category = "Anansi|Checkpoint")
	void Activate();

	UPROPERTY(BlueprintAssignable, Category = "Anansi|Checkpoint")
	FOnCheckpointActivated OnCheckpointActivated;

	/** Display name for HUD notification. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anansi|Checkpoint")
	FText CheckpointName = NSLOCTEXT("Anansi", "DefaultCP", "Checkpoint");

	/** Get the respawn location (slightly above the checkpoint). */
	UFUNCTION(BlueprintPure, Category = "Anansi|Checkpoint")
	FVector GetRespawnLocation() const;

	/** Static: get the most recently activated checkpoint in the world. */
	UFUNCTION(BlueprintPure, Category = "Anansi|Checkpoint", meta = (WorldContext = "WorldContext"))
	static ACheckpoint* GetActiveCheckpoint(UObject* WorldContext);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PillarMesh;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* Comp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bIsActivated = false;

	static TWeakObjectPtr<ACheckpoint> ActiveCheckpoint;
};
