// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItemSlot.generated.h"

class UImage;
class UTextBlock;
class UBorder;
class UInventoryTooltip;
class UDragItemVisual;
class UItemBase;
/**
 * 
 */
UCLASS()
class DRG2_API UInventoryItemSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	FORCEINLINE void SetItemReference(UItemBase* ItemIn){ItemReference = ItemIn;}
	FORCEINLINE UItemBase* GetItemReference() const {return ItemReference;}
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Item Slot")
	TSubclassOf<UDragItemVisual> DragItemVisualClass;
	UPROPERTY(EditDefaultsOnly, Category = "Item Slot")
	TSubclassOf<UInventoryTooltip> ToolTipClass;
	UPROPERTY(VisibleAnywhere,Category = "Inventory Slot")
	UItemBase* ItemReference;
	UInventoryTooltip* ToolTipReference;
	
	UPROPERTY(VisibleAnywhere,Category = "Inventory Slot",meta = (BindWidget))
	UBorder* ItemBorder;
	UPROPERTY(VisibleAnywhere,Category = "Inventory Slot",meta = (BindWidget))
	UImage* ItemIcon;
	UPROPERTY(VisibleAnywhere,Category = "Inventory Slot",meta = (BindWidget))
	UTextBlock* ItemQuantity;

	void UpdateSlot();
	
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};
