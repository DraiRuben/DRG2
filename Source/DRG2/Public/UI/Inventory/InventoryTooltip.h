// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryTooltip.generated.h"

class UTextBlock;
class UInventoryItemSlot;
/**
 * 
 */
UCLASS()
class DRG2_API UInventoryTooltip : public UUserWidget
{
	GENERATED_BODY()
public:
	UInventoryItemSlot* InventorySlotBeingHovered;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemName;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemType;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageValue;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ArmorValue;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemDescription;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StackSizeText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxStackSize;

	void UpdateTooltip() const;
protected:
	virtual void NativeConstruct() override;
};
