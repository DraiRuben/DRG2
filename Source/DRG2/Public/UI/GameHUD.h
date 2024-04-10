// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainMenuWidget.h"
#include "GameFramework/HUD.h"
#include "Inventory/InteractionWidget.h"
#include "Inventory/InventoryPanel.h"
#include "Items/Interfaces/Interactable.h"
#include "GameHUD.generated.h"

/**
 * 
 */
UCLASS()
class DRG2_API AGameHUD : public AHUD
{
	GENERATED_BODY()
public:
	AGameHUD();
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UInventoryPanel> InventoryPanelWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UInteractionWidget> InteractionWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass; 
	virtual void BeginPlay() override;

	void ShowInventory();
	void HideInventory();
	void ShowInteractionWidget() const;
	void HideInteractionWidget() const;
	void UpdateInteractionWidget(const FInteractableData* InteractableData) const;
	bool IsInventoryVisible;

private:
	UInventoryPanel* InventoryPanel;
	UInteractionWidget* InteractionWidget;
};
