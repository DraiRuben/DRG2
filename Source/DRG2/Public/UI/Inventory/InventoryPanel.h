// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/UniformGridPanel.h"
#include "InventoryPanel.generated.h"

/**
 * 
 */
UCLASS()
class DRG2_API UInventoryPanel : public UUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "Inventory Widget | Slot")
	UHorizontalBox* HotBar;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "Inventory Widget | Slot")
	UUniformGridPanel* InventoryStorage;
public:
	void HideInventory();
	void ShowInventory();
};
