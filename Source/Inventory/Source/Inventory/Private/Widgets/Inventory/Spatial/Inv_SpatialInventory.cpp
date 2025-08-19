// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_SpatialInventory.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "Components/Button.h"

void UInv_SpatialInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Equippables->OnClicked.AddDynamic(this,&ThisClass::ShowEquippables);
	Button_Comsumables->OnClicked.AddDynamic(this,&ThisClass::ShowComsumables);
	Button_Craftables->OnClicked.AddDynamic(this,&ThisClass::ShowCraftables);
}

FInv_SlotAvailabilityResult UInv_SpatialInventory::HasRoomForItem(UInv_ItemComponent* ItemComponent) const
{
	FInv_SlotAvailabilityResult Result;
	Result.TotalRoomToFill = 1;
	return Result;
}

void UInv_SpatialInventory::ShowEquippables()
{
	SetActiveGrid(Grid_Equippables,Button_Equippables);
}

void UInv_SpatialInventory::ShowComsumables()
{
	SetActiveGrid(Grid_Consumables,Button_Comsumables);
}

void UInv_SpatialInventory::ShowCraftables()
{
	SetActiveGrid(Grid_Craftables,Button_Craftables);
}

void UInv_SpatialInventory::DisableButton(UButton* Button)
{
	Button_Equippables->SetIsEnabled(true);
	Button_Comsumables->SetIsEnabled(true);
	Button_Craftables->SetIsEnabled(true);
	Button->SetIsEnabled(false);
}

void UInv_SpatialInventory::SetActiveGrid(UInv_InventoryGrid* Grid, UButton* Button)
{
	DisableButton(Button);

	Switcher->SetActiveWidget(Grid);
}
