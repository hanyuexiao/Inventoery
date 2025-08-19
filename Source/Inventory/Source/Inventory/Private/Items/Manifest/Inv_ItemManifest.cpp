#include "Items/Manifest/Inv_ItemManifest.h"

#include "Items/Inv_InventoryItem.h"

//复印的过程
UInv_InventoryItem* FInv_ItemManifest::Manifest(UObject* NewOuter)
{
	//   调用UE的全局函数NewObject<T>()来在内存中实例化一个新的UInv_InventoryItem。
	// - NewOuter: 指定了这个新UObject的“所有者”，这对于UE的垃圾回收(GC)至关重要。
	// - UInv_InventoryItem::StaticClass(): 告诉NewObject我们要创建的具体类型。
	UInv_InventoryItem* Item = NewObject<UInv_InventoryItem>(NewOuter,UInv_InventoryItem::StaticClass());
	Item->SetItemManifest(*this);

	return Item;
}
