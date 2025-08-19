// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventorymanagement/Utils/Inv_InventoryStatics.h"

#include "Inventorymanagement/Components/Inv_InventoryComponent.h"

UInv_InventoryComponent* UInv_InventoryStatics::GetInventoryComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController)) return nullptr;
	UInv_InventoryComponent* InventoryComponent = PlayerController -> FindComponentByClass<UInv_InventoryComponent>();
	return InventoryComponent;
}
