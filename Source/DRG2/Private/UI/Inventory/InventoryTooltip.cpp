// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/InventoryTooltip.h"

#include "Components/TextBlock.h"
#include "Items/ItemBase.h"
#include "UI/Inventory/InventoryItemSlot.h"

void UInventoryTooltip::UpdateTooltip() const
{
	if(UItemBase* ItemBeingHovered = InventorySlotBeingHovered->GetItemReference())
	{
		switch(ItemBeingHovered->ItemType)
		{
        	case EItemType::Armor:
        		break;
        	case EItemType::Weapon:
        		break;
        	case EItemType::Consumable:
        		break;
        	case EItemType::Block:
        		DamageValue->SetVisibility(ESlateVisibility::Collapsed);
        		ArmorValue->SetVisibility(ESlateVisibility::Collapsed);
        		ItemType->SetText(FText::FromString("Block"));
        		break;
        	case EItemType::Misc:
        		break;
		}
        if(ItemBeingHovered->NumericData.IsStackable)
        {
        	StackSizeText->SetText(FText::AsNumber(ItemBeingHovered->NumericData.StackLimit));
        }
        else
        {
        	StackSizeText->SetVisibility(ESlateVisibility::Collapsed);
        	MaxStackSize->SetVisibility(ESlateVisibility::Collapsed);
        }
        ItemName->SetText(ItemBeingHovered->DescriptionData.Name);
        ItemDescription->SetText(ItemBeingHovered->DescriptionData.Description);
	}
	
}

void UInventoryTooltip::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateTooltip();
	
}
