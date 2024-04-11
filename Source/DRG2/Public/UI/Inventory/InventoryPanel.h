// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/UniformGridPanel.h"
#include "InventoryPanel.generated.h"

class UInventoryItemSlot;
class UInventoryComponent;
class APlayerCharacter;
/**
 * 
 */
UCLASS()
class DRG2_API UInventoryPanel : public UUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "Inventory Widget | Slot")
	UUniformGridPanel* HotBar;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "Inventory Widget | Slot")
	UUniformGridPanel* InventoryStorage;
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
public:
	void HideInventory();
	void ShowInventory();
	void RefreshInventory();

	UPROPERTY()
	APlayerCharacter* PlayerCharacter;
	UPROPERTY()
	UInventoryComponent* InventoryReference;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryItemSlot> InventorySlotClass;
	int32 GetMouseDropPositionAsInventorySlotIndex();
};
