// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryPanel.h"
#include "MainMenuWidget.generated.h"

class APlayerCharacter;
/**
 * 
 */
UCLASS()
class DRG2_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
	UPROPERTY()
	APlayerCharacter* PlayerCharacter;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "Inventory Widget")
	UInventoryPanel* InventoryWidget;
};
