// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Inventorymanagement/Components/Inv_InventoryComponent.h"
#include "Inventorymanagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnItemAdded.AddDynamic(this,&ThisClass::AddItem);
	
}


void UInv_InventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	if (!MatchesCategory(Item)) return;

	FInv_SlotAvailabilityResult Result = HasRoomForItem(Item);
	AddItemToIndices(Result, Item);
}

bool UInv_InventoryGrid::MatchesCategory(const UInv_InventoryItem* Item) const
{
	// 有三个种类的Grid背包 -> 为了知道所拾取的道具是哪一个背包的物品，所以先检查其catagory,判断其属于哪一类
	// 为了获取其catagory，所以需要获取物品的清单，读取清单来查看属于哪个catagory
	// 在蓝图里创建了三个种类的Grid，每个Grid都有MatchesCategory函数
	// 当广播的时候，会自动调用AddItem(UInv_InventoryItem* Item)，所以每一个种类的背包都会调用该函数
	// 只有与所拾取物品的种类相同的背包才会进行下一步逻辑，去询问该种类背包是否还含有空间
	return Item->GetItemManifest().GetItemCategory() == ItemCategory;
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_ItemComponent* ItemComponent)
{
	return HasRoomForItem(ItemComponent->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_InventoryItem* Item)
{
	return HasRoomForItem(Item->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const FInv_ItemManifest& Manifest)
{
	FInv_SlotAvailabilityResult Result;
	
	// Determine if the item is stackable.
	const FInv_StackableFragment* StackableFragment = Manifest.GetFragmentOfType<FInv_StackableFragment>();
	Result.bStackable = StackableFragment != nullptr;
	
	// Determine how many stacks to add.
	const int32 MaxStackSize = StackableFragment ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = StackableFragment ? StackableFragment -> GetStackCount() : 1;

	
	TSet<int32> CheckedIndices;
	// For each Grid Slot:
	for (const auto& GridSlot : GridSlots)
	{
		// If we don't have anymore to fill, break out of the loop early.
		if (AmountToFill == 0) break;
		
		// Is this index claimed yet?
		if (IsIndexClaimed(CheckedIndices,GridSlot->GetIndex())) continue;

		
		// Can the item fit here? (i.e. is it out of grid bounds?)
		if (!IsInGridBounds(GridSlot->GetIndex(),GetItemDimensions(Manifest))) continue;
		TSet<int32> TentativelyClaimed;
		if (!HasRoomAtIndex(GridSlot,GetItemDimensions(Manifest),CheckedIndices,TentativelyClaimed,Manifest.GetItemType(),MaxStackSize))
		{
			continue;
		}
		
		// How much to fill?
		const int32 AmountToFillInSlot = DetermineFillAmountForSlot(Result.bStackable,MaxStackSize,AmountToFill,GridSlot);
		if (AmountToFillInSlot == 0) continue;

		CheckedIndices.Append(TentativelyClaimed);
		
		// Update the amount left to fill
		Result.TotalRoomToFill += AmountToFillInSlot;
		Result.SlotAvailabilities.Emplace(
			FInv_SlotAvaliability
			{
				HasValidItem(GridSlot) ? GridSlot -> GetUpperLeftIndex() : GridSlot->GetIndex(),
				Result.bStackable ? AmountToFillInSlot : 0,
				HasValidItem(GridSlot)
			}
		);
		AmountToFill -= AmountToFillInSlot;
		
		// How much is the Remainder?
		Result.Remainder = AmountToFill;
		
		if (AmountToFill == 0) return Result;
	}



	return Result;
}

bool UInv_InventoryGrid::HasRoomAtIndex(const UInv_GridSlot* GridSlot,
										const FIntPoint& Dimensions,
										const TSet<int32>& CheckedIndices,
										TSet<int32>& OutTentativelyClaimed,
										const FGameplayTag& ItemType,
										const int32 MaxStackSize)
{
	bool bHasRoomAtIndex = true;

	UInv_InventoryStatics::ForEach2D(GridSlots,GridSlot->GetIndex(),Dimensions,Columns,[&](const UInv_GridSlot* SubGridSlot)
	{
		if (CheckSlotConstraints(GridSlot,SubGridSlot,CheckedIndices,OutTentativelyClaimed,ItemType,MaxStackSize))
		{
			OutTentativelyClaimed.Add(SubGridSlot->GetIndex());
		}
		else
		{
			bHasRoomAtIndex = false;
		}
	});
	return bHasRoomAtIndex;
}

bool UInv_InventoryGrid::CheckSlotConstraints(const UInv_GridSlot* GridSlot,
                                              const UInv_GridSlot* SubGridSlot,
	                                          const TSet<int32>& CheckedIndices,
	                                          TSet<int32>& OutTentativelyClaimed,
	                                          const FGameplayTag& ItemType,
	                                          const int32 MaxStackSize) const
{
	// Index claimed?
	if (IsIndexClaimed(CheckedIndices,SubGridSlot->GetIndex())) return false;

	// Has valid item?
	if (!HasValidItem(SubGridSlot))
	{
		OutTentativelyClaimed.Add(SubGridSlot->GetIndex());
		return true;
	}
	
	// Is this Grid Slot an upper left slot?
	if (!IsUpperLeftSlot(GridSlot,SubGridSlot)) return false;

	// If so, is this a stackable item?
	const UInv_InventoryItem* SubItem = SubGridSlot->GetInventoryItem().Get();
	if (!SubItem->IsStackable()) return false;
	
	// Is this item the same type as the item we're trying to add?
	if (!DoesItemTypeMatch(SubItem,ItemType)) return false;
	
	// If stackable, is this slot at the max stack size already?
	if (GridSlot->GetStackCount() >= MaxStackSize) return false;
	
	return true;
}

FIntPoint UInv_InventoryGrid::GetItemDimensions(const FInv_ItemManifest& Manifest) const
{
	const FInv_GridFragment* GridFragment  = Manifest.GetFragmentOfType<FInv_GridFragment>();
	return GridFragment? GridFragment -> GetGridSize() : FIntPoint(1, 1);
}

bool UInv_InventoryGrid::HasValidItem(const UInv_GridSlot* GridSlot) const
{
	return GridSlot->GetInventoryItem().IsValid();
}

bool UInv_InventoryGrid::IsUpperLeftSlot(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot) const
{
	return SubGridSlot->GetUpperLeftIndex() == GridSlot->GetIndex();
}

bool UInv_InventoryGrid::DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const
{
	return SubItem->GetItemManifest().GetItemType() == ItemType;
}

bool UInv_InventoryGrid::IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const 
{
	if (StartIndex < 0 || StartIndex <= GridSlots.Num()) return false;
	const int32 EndColumn = (StartIndex % Columns) + ItemDimensions.X;
	const int32 EndRow = (StartIndex / Columns) + ItemDimensions.Y;
	return EndColumn <= Columns && EndRow <= Rows;
}

int32 UInv_InventoryGrid::DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize,const int32 AmountToFill, const UInv_GridSlot* GridSlot) const
{
	// calculate room in the slot
	const int32 RoomInSlot = MaxStackSize - GetStackAmount(GridSlot);
	return bStackable ? FMath::Min(AmountToFill,RoomInSlot) : 1;
	// if stackable,need the minimum between AmountFill and RoomInSlot
}

int32 UInv_InventoryGrid::GetStackAmount(const UInv_GridSlot* GridSlot) const
{
	int32 CurrentSlotStackCount = GridSlot->GetStackCount();
	// if we are at a slot that doesn't hold the stack count.we must get the actual stack count.
	if (const int32 UpperLeftIndex = GridSlot->GetUpperLeftIndex(); UpperLeftIndex != INDEX_NONE)
	{
		UInv_GridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
		CurrentSlotStackCount = UpperLeftGridSlot -> GetStackCount();
	}
	return CurrentSlotStackCount;
}

void UInv_InventoryGrid::AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem)
{
	for (const auto& Availability : Result.SlotAvailabilities)
	{
		AddItemAtIndex(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
		UpdateGridSlots(NewItem, Availability.Index,Result.bStackable, Availability.AmountToFill);
	}
}

void UInv_InventoryGrid::AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable, const int32 StackAmount)
{
	// 占用空间插件
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	// 物品图标插件
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(Item, FragmentTags::IconFragment);
	
	if (!GridFragment || !ImageFragment) return;

	UInv_SlottedItem* SlottedItem = CreateSlottedItem(Item, bStackable, StackAmount, GridFragment, ImageFragment, Index);

	// Add the slotted item to the canvas panel.
	AddSlottedItemToCanvas(Index, GridFragment, SlottedItem);
	// Store the new widget in a container.
	SlottedItems.Add(Index, SlottedItem);
}

UInv_SlottedItem* UInv_InventoryGrid::CreateSlottedItem(UInv_InventoryItem* Item, const bool bStackable, const int32 StackAmount, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment, const int32 Index)
{
	UInv_SlottedItem* SlottedItem = CreateWidget<UInv_SlottedItem>(GetOwningPlayer(), SlottedItemClass);
	
	SlottedItem->SetInventoryItem(Item);
	SetSlottedItemImage(SlottedItem, GridFragment, ImageFragment);
	SlottedItem->SetGridIndex(Index);
	SlottedItem->SetIsStackable(bStackable);
	const int32 StackUpdateAmount = bStackable ? StackAmount : 0;
	SlottedItem->UpdateStackCount(StackUpdateAmount);
	
	return SlottedItem;
}

void UInv_InventoryGrid::SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment,
	const FInv_ImageFragment* ImageFragment) const
{
	// Image只是一个画框。内部需要一个FSlateBrush数据去告诉它需要画什么，而 FSlateBrush 是不可见的
	FSlateBrush Brush;
	// 从物品上获取它在网格上的Image
	Brush.SetResourceObject(ImageFragment->GetIcon());
	// 
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = GetDrawSize(GridFragment);
	SlottedItem->SetImageBrush(Brush);
}

void UInv_InventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment, UInv_SlottedItem* SlottedItem) const
{
	CanvasPanel->AddChild(SlottedItem);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem);
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	const FVector2D DrawPos = UInv_WidgetUtils::GetPositionFromIndex(Index, Columns) * TileSize;
	const FVector2D DrawPosWithPadding = DrawPos + FVector2D(GridFragment->GetGridPadding());
	CanvasSlot->SetPosition(DrawPosWithPadding);
}

void UInv_InventoryGrid::UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index,bool bStackableItem,const int32 StackAmount)
{
	check(GridSlots.IsValidIndex(Index));
	// 传入的第一个Index是物品的初始坐标所以肯定是左上角的坐标位置，且没放入循环中，就实现了stack值只存放在左上角
	if (bStackableItem)
	{
		GridSlots[Index] -> SetStackCount(StackAmount);
	}
	
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(NewItem, FragmentTags::GridFragment);
	const FIntPoint Dimensions = GridFragment ? GridFragment -> GetGridSize() : FIntPoint(1,1);

	UInv_InventoryStatics::ForEach2D(GridSlots,Index,Dimensions,Columns,[&](UInv_GridSlot* GridSlot)
	{
		GridSlot -> SetInventoryItem(NewItem);
		GridSlot -> SetUpperLeftIndex(Index);
		GridSlot -> SetOccupiedTexture();
		GridSlot -> SetAvailable(false);
	});
	
	
}

bool UInv_InventoryGrid::IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const
{
	return CheckedIndices.Contains(Index);
}

FVector2D UInv_InventoryGrid::GetDrawSize(const FInv_GridFragment* GridFragment) const
{
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	return GridFragment->GetGridSize() * IconTileWidth;
}

void UInv_InventoryGrid::ConstructGrid()
{
	GridSlots.Reserve(Rows * Columns);

	for(int32 j = 0;j < Rows; ++j)
	{
		for (int32 i = 0; i < Columns; ++i)
		{
			UInv_GridSlot* GridSlot = CreateWidget<UInv_GridSlot>(this,GridSlotClass);
			CanvasPanel->AddChild(GridSlot);

			const FIntPoint TilePosition(i,j);
			GridSlot->SetTileIndex(UInv_WidgetUtils::GetIndexFromPosition(FIntPoint(i,j),Columns));
                                   			
			UCanvasPanelSlot* GridCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			GridCPS -> SetSize(FVector2D(TileSize));
			GridCPS -> SetPosition(TilePosition * TileSize);

			GridSlots.Add(GridSlot);
		}
	}
}




