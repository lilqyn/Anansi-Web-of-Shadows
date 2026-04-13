// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InventorySystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, FName, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, FName, ItemID);

/**
 * UInventorySystem
 *
 * Simple key-value inventory. Tracks item IDs and counts.
 * Used for keys, quest items, currency, etc.
 */
UCLASS()
class ANANSIGAME_API UInventorySystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Anansi|Inventory")
	void AddItem(FName ItemID, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Anansi|Inventory")
	bool RemoveItem(FName ItemID, int32 Count = 1);

	UFUNCTION(BlueprintPure, Category = "Anansi|Inventory")
	bool HasItem(FName ItemID, int32 MinCount = 1) const;

	UFUNCTION(BlueprintPure, Category = "Anansi|Inventory")
	int32 GetItemCount(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Anansi|Inventory")
	TArray<FName> GetAllItemIDs() const;

	UPROPERTY(BlueprintAssignable) FOnItemAdded OnItemAdded;
	UPROPERTY(BlueprintAssignable) FOnItemRemoved OnItemRemoved;

private:
	UPROPERTY()
	TMap<FName, int32> Items;
};
