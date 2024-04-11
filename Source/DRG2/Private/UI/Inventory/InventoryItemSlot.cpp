// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/InventoryItemSlot.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Items/ItemBase.h"
#include "UI/Inventory/DragItemVisual.h"
#include "UI/Inventory/InventoryTooltip.h"
#include "UI/Inventory/ItemDragDropOperation.h"

void UInventoryItemSlot::UpdateSlot()
{
	if(ItemReference)
	{
		switch(ItemReference->ItemRarity) {
		case EItemRarity::Bad:
			break;
		case EItemRarity::Common:
			ItemBorder->SetBrushColor(FLinearColor(0.15f,0.15f,0.15f));
			break;
		case EItemRarity::Rare:
			break;
		case EItemRarity::Epic:
			break;
		case EItemRarity::Legendary:
			break;
		}
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
		ItemIcon->SetBrushFromTexture(ItemReference->AssetData.Icon);
		if(ItemReference->NumericData.IsStackable)
		{
			ItemQuantity->SetText(FText::AsNumber(ItemReference->Quantity));
		}
		else
		{
			ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
	ToolTipReference->UpdateTooltip();
	SetToolTip(ItemReference?ToolTipReference:nullptr);
	
}

void UInventoryItemSlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if(ToolTipClass)
	{
		ToolTipReference = CreateWidget<UInventoryTooltip>(this,ToolTipClass);
		ToolTipReference->InventorySlotBeingHovered = this;
		SetToolTip(ItemReference?ToolTipReference:nullptr);
	}
}

void UInventoryItemSlot::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateSlot();
}

FReply UInventoryItemSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry,InMouseEvent);
	if(InMouseEvent.GetEffectingButton()==EKeys::LeftMouseButton)
	{
		return Reply.Handled().DetectDrag(TakeWidget(),EKeys::LeftMouseButton);
	}
	return Reply.Unhandled();
}
void UInventoryItemSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UInventoryItemSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if(DragItemVisualClass && ItemReference)
	{
		UDragItemVisual* DragVisual = CreateWidget<UDragItemVisual>(this,DragItemVisualClass);
		DragVisual->ItemIcon->SetBrushFromTexture(ItemReference->AssetData.Icon);
		DragVisual->ItemQuantity->SetText(FText::AsNumber(ItemReference->Quantity));

		UItemDragDropOperation* DragItemOperation = NewObject<UItemDragDropOperation>();
		DragItemOperation->SourceItem = ItemReference;
		DragItemOperation->SourceInventory = ItemReference->OwningInventory;
		DragItemOperation->DefaultDragVisual = DragVisual;
		DragItemOperation->Pivot = EDragPivot::CenterCenter;
		OutOperation = DragItemOperation;
	}
}

bool UInventoryItemSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

