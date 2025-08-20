#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Types/AttributeStorage.h"

#include "Inv_ItemFragment.generated.h"

USTRUCT()
struct FInv_ItemFragment
{
	GENERATED_BODY()

	FInv_ItemFragment() {}
	//拷贝构造函数
	FInv_ItemFragment(const FInv_ItemFragment&) = default;
	//拷贝赋值运算符 (Copy Assignment Operator)
	FInv_ItemFragment& operator=(const FInv_ItemFragment&) = default;
	
	//移动构造函数
	FInv_ItemFragment(FInv_ItemFragment&&) = default;
	//移动赋值运算函数(Move Assignment Operator)
	FInv_ItemFragment& operator=(FInv_ItemFragment&&) = default;
	
	virtual ~FInv_ItemFragment() {}

	FGameplayTag GetFragmentTag() const {return FragmentTag;}
	void SetFragmentTag(FGameplayTag Tag) {FragmentTag = Tag;}
private:

	UPROPERTY(EditAnywhere,Category = "Inventory")
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
};



USTRUCT(BlueprintType)
struct FInv_GridFragment:public FInv_ItemFragment
{
	GENERATED_BODY()

	FIntPoint GetGridSize() const {return GridSize;}
	void SetGripSize(const FIntPoint& Size) {GridSize = Size;}
	float GetGridPadding() const {return GridPadding;}
	void SetGripPadding(float Padding) {GridPadding = Padding;}

private:

	UPROPERTY(EditAnywhere,Category = "Inventory")
	FIntPoint GridSize{1,1};

	UPROPERTY(EditAnywhere,Category = "Inventory")
	float GridPadding{0.f};
};



USTRUCT(BlueprintType)
struct FInv_ImageFragment : public FInv_ItemFragment
{
	GENERATED_BODY()

	UTexture2D* GetIcon() const { return Icon; }

private:

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TObjectPtr<UTexture2D> Icon{nullptr};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FVector2D IconDimensions{44.f, 44.f};
};











