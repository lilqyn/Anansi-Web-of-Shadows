// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

#include "Core/InventorySystem.h"
#include "AnansiGame.h"

void UInventorySystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UInventorySystem::AddItem(FName ItemID, int32 Count)
{
	if (Count <= 0) return;
	Items.FindOrAdd(ItemID) += Count;
	OnItemAdded.Broadcast(ItemID);
	UE_LOG(LogAnansi, Log, TEXT("Inventory: +%d %s (total: %d)"), Count, *ItemID.ToString(), Items[ItemID]);
}

bool UInventorySystem::RemoveItem(FName ItemID, int32 Count)
{
	int32* Current = Items.Find(ItemID);
	if (!Current || *Current < Count) return false;

	*Current -= Count;
	if (*Current <= 0) Items.Remove(ItemID);
	OnItemRemoved.Broadcast(ItemID);
	return true;
}

bool UInventorySystem::HasItem(FName ItemID, int32 MinCount) const
{
	const int32* Count = Items.Find(ItemID);
	return Count && *Count >= MinCount;
}

int32 UInventorySystem::GetItemCount(FName ItemID) const
{
	const int32* Count = Items.Find(ItemID);
	return Count ? *Count : 0;
}

TArray<FName> UInventorySystem::GetAllItemIDs() const
{
	TArray<FName> Keys;
	Items.GetKeys(Keys);
	return Keys;
}
