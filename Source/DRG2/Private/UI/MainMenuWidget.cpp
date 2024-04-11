// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenuWidget.h"

#include "PlayerCharacter.h"
#include "Items/ItemBase.h"
#include "UI/Inventory/ItemDragDropOperation.h"

void UMainMenuWidget::NativeOnInitialized()
{
	UUserWidget::NativeOnInitialized();
}

void UMainMenuWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();
	PlayerCharacter = Cast<APlayerCharacter>(GetOwningPlayerPawn());
}

bool UMainMenuWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);
	if(PlayerCharacter && ItemDragDrop->SourceItem)
	{
		PlayerCharacter->DropItem(ItemDragDrop->SourceItem,ItemDragDrop->SourceItem->Quantity);
		return true;
	}
	return false;
}
