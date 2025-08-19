// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Components/Inv_InventoryComponent.h"

#include "Blueprint/UserWidget.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"


UInv_InventoryComponent::UInv_InventoryComponent() : InventoryList(this)
{

	PrimaryComponentTick.bCanEverTick = false;
	// 第1行: 确保组件本身会被复制
	SetIsReplicatedByDefault(true);
	// 第2行: 【关键开关】将这个布尔值设为true，就等于告诉引擎：
	// “除了我自己的属性，请同时留意我手动注册的一个子对象列表，并帮我同步它们。”
	bReplicateUsingRegisteredSubObjectList = true;
	bInventoryMenuOpen = false;
}

void UInv_InventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass,InventoryList);
}

void UInv_InventoryComponent::TryAddItem(UInv_ItemComponent* ItemComponent)
{
	//本地检查
	FInv_SlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComponent);

	if (Result.TotalRoomToFill == 0)
	{
		NoRoomInInventory.Broadcast();
		return;
	}

	if (Result.ExistingItem.IsValid() && Result.bStackeable)
	{
		//Add stacks to an item that already exists in the inventory.We only want to update the stack count.
		//not creat a new item of this type
		Server_AddStacksToItem(ItemComponent,Result.TotalRoomToFill,Result.Remainder);
	}
	else if (Result.TotalRoomToFill > 0)
	{
		//创建新的背包网格给新物品
		//This item type doesn't exist in the inventory. Creat a new one and update all pertinent slots
		Server_AddNewItem(ItemComponent,Result.bStackeable ? Result.TotalRoomToFill : 0);
	}
	
}

void UInv_InventoryComponent::Server_AddNewItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount)
{
	UInv_InventoryItem* NewItem  = InventoryList.AddEntry(ItemComponent);

	if (GetOwner() -> GetNetMode() == NM_ListenServer || GetOwner() -> GetNetMode() == NM_Standalone)
	{
		OnItemAdded.Broadcast(NewItem);
	}
	
	//TODO: Tell the Item Component to destroy its owning actor.
}

void UInv_InventoryComponent::Server_AddStacksToItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount,
	int32 Remainder)
{
	
}

void UInv_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ConstructInventory();
}

void UInv_InventoryComponent::ToggleInventoryMenu()
{
	if(bInventoryMenuOpen)
	{
		CloseInventoryMenu();
	}
	else
	{
		OpenInventoryMenu();
	}
}

void UInv_InventoryComponent::AddRepSubobj(UObject* SubObj)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		//这个函数又是什么作用？ why
		AddReplicatedSubObject(SubObj);
	}
}

void UInv_InventoryComponent::ConstructInventory()
{
	OwningController = Cast<APlayerController>(GetOwner());
	checkf(OwningController.IsValid(),TEXT("Inventory Component should have a Player Controller as Owner"));
	if(!OwningController->IsLocalController()) return;

	InventoryMenu = CreateWidget<UInv_InventoryBase>(OwningController.Get(),InventoryMenuClass);
	InventoryMenu->AddToViewport();
	CloseInventoryMenu();
}

void UInv_InventoryComponent::OpenInventoryMenu()
{
	if(!IsValid(InventoryMenu)) return;

	//TODO Open the Inventory
	InventoryMenu->SetVisibility(ESlateVisibility::Visible);
	bInventoryMenuOpen = true;

	//TODO want to show the mouse
	if(!OwningController.IsValid()) return;

	FInputModeGameAndUI InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(true);
}

void UInv_InventoryComponent::CloseInventoryMenu()
{
	if(!IsValid(InventoryMenu)) return;

	InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
	bInventoryMenuOpen = false;

	if(!OwningController.IsValid()) return;
	
	FInputModeGameOnly InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(false);
}





