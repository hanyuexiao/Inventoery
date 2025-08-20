#pragma once

#include "Inv_GridTypes.generated.h"

class UInv_InventoryItem;

UENUM(BlueprintType)
enum class EInv_ItemCategory : uint8
{
	Equippable,
	Consumable,
	Craftable,
	None
};

USTRUCT()
struct FInv_SlotAvaliability
{
	GENERATED_BODY()

	FInv_SlotAvaliability(){}
	FInv_SlotAvaliability(int32 ItemIndex,int32 Room,bool bHasItem) : Index(ItemIndex),AmountToFill(Room),bItemAtIndex(bHasItem){}

	//最小单元格的位置，标识哪一个格子 -> 在哪里操作
	int32 Index{INDEX_NONE};
	//计划放入这个格子的Item数量 -> 操作什么
	int32 AmountToFill{0};
	//这个格子是否已经有Item; -> 如何操作
	bool bItemAtIndex{false};
};

USTRUCT()
struct FInv_SlotAvailabilityResult
{
	GENERATED_BODY()

	FInv_SlotAvailabilityResult() {}

	//用弱指针指向已经在背包存在的物品
	//Why use Weak?
	TWeakObjectPtr<UInv_InventoryItem> ExistingItem;
	
	//总共能放下多少Item
	int32 TotalRoomToFill{0};
	//最总还剩下多少Item放不下
	int32 Remainder{0};
	//该Item是否可以堆叠？
	bool bStackable{false};
	TArray<FInv_SlotAvaliability> SlotAvailabilities;
};
