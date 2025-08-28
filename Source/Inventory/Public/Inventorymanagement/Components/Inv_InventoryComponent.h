﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventorymanagement/FastArray/Inv_FastArray.h"
#include "Inv_InventoryComponent.generated.h"


struct FInv_SlotAvailabilityResult;
class UInv_InventoryItem;
class UInv_InventoryBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChange,UInv_InventoryItem*,Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoRoomInInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStackChange,const FInv_SlotAvailabilityResult&,Result);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),Blueprintable)
class INVENTORY_API UInv_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInv_InventoryComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	
	UFUNCTION(BlueprintCallable,BlueprintAuthorityOnly,Category= "Inventory")
	void TryAddItem(UInv_ItemComponent* ItemComponent);

	//-----------------------与服务器沟通交互------------------------------------//
	UFUNCTION(Server,Reliable)
	void Server_AddNewItem(UInv_ItemComponent* ItemComponent,int32 StackCount);

	UFUNCTION(Server,Reliable)
	void Server_AddStacksToItem(UInv_ItemComponent* ItemComponent,int32 StackCount,int32 Remainder);
	//-----------------------------------------------------------------------//
	void ToggleInventoryMenu();
	void AddRepSubobj(UObject* SObject);
	
	FInventoryItemChange OnItemAdded;
	FInventoryItemChange OnItemRemoved;
	FNoRoomInInventory NoRoomInInventory;
	FStackChange OnStackChange;
	
protected:
	virtual void BeginPlay() override;


	
private:
	TWeakObjectPtr<APlayerController> OwningController;
	
	void ConstructInventory();

	UPROPERTY(Replicated)
	FInv_InventoryFastArray InventoryList;
	
	UPROPERTY()
	TObjectPtr<UInv_InventoryBase> InventoryMenu;
	
	UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UInv_InventoryBase> InventoryMenuClass;

	//-----------------------Have Open The Inventory? ------------------------//
	
	bool bInventoryMenuOpen = true;
	
	void OpenInventoryMenu();

	void CloseInventoryMenu();
	
	//-------------------------------------------------------------------------//
};
