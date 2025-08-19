// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_InventoryItem.generated.h"

struct FInstancedStruct;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryItem : public UObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override {return true;}
	
	void SetItemManifest(const FInv_ItemManifest& Manifest);

	const FInv_ItemManifest& GetItemManifest() const {return ItemManifest.Get<FInv_ItemManifest>();}

	FInv_ItemManifest& GetItemManifestMutable() {return ItemManifest.GetMutable<FInv_ItemManifest>();}
private:
	// FInstancedStruct是什么？ 什么时候会想到使用FInstancedStruct
	// 与TSubclassOf有什么区别
	UPROPERTY(VisibleAnywhere,meta = (BaseStruct = "/Script/Inventory.Inv_ItemManifest"),Replicated)
	FInstancedStruct ItemManifest;
};
