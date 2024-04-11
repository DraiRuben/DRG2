// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemBase.h"

#include "Items/InventoryComponent.h"

UItemBase::UItemBase() : IsCopy(false), IsPickup(false)
{
}

void UItemBase::ResetItemFlags()
{
	IsCopy = false;
	IsPickup = false;
}
UItemBase* UItemBase::CreateItemCopy() const
{
	UItemBase* ItemCopy = NewObject<UItemBase>(StaticClass());
	ItemCopy->ID = this->ID;
	ItemCopy->Quantity = this->Quantity;
	ItemCopy->ItemRarity = this->ItemRarity;
	ItemCopy->ItemType = this->ItemType;
	ItemCopy->Statistics = this->Statistics;
	ItemCopy->AssetData = this->AssetData;
	ItemCopy->NumericData = this->NumericData;
	ItemCopy->DescriptionData = this->DescriptionData;
	ItemCopy->IsCopy = true;
	return ItemCopy;
}

void UItemBase::SetQuantity(const int32 NewQuantity)
{
	if(NewQuantity != Quantity)
	{
		Quantity = FMath::Clamp(NewQuantity,0,NumericData.IsStackable?NumericData.StackLimit:1);
		if(OwningInventory)
		{
			if(Quantity<=0)
			{
				OwningInventory->RemoveSingleInstanceOfItem(this);
			}
		}
	}
}

void UItemBase::Use(ACharacter* Character)
{
}
