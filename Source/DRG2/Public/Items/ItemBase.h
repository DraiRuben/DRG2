// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemData.h"
#include "ItemBase.generated.h"

/**
 * 
 */
UCLASS()
class DRG2_API UItemBase : public UObject
{
	GENERATED_BODY()
public:
	//UPROPERTY()
	//UInventoryComponent* OwningInventory;
	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	int32 Quantity;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FName ID;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	EItemRarity ItemRarity;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	EItemType ItemType;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemStatistics Statistics;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemAssetData AssetData;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemDescriptionData DescriptionData;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemNumericData NumericData;

	UItemBase();
	UFUNCTION(Category = "Item")
	UItemBase* CreateItemCopy() const;
	UFUNCTION(Category = "Item")
	FORCEINLINE bool IsFullItemStack() const {return Quantity == NumericData.StackLimit;};
	UFUNCTION(Category = "Item")
	void SetQuantity(const int32 NewQuantity);
	UFUNCTION(Category = "Item")
	virtual void Use(ACharacter* Character);
protected:
	bool operator==(const FName& OtherID) const
	{
		return this->ID == OtherID;
	}
};
