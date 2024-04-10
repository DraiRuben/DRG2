// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/InventoryPanel.h"

void UInventoryPanel::HideInventory()
{
	InventoryStorage->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryPanel::ShowInventory()
{
	InventoryStorage->SetVisibility(ESlateVisibility::Visible);
}
