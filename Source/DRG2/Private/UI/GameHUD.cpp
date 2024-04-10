// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameHUD.h"

AGameHUD::AGameHUD()
{
}

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();
	if(InventoryPanelWidgetClass)
	{
		InventoryPanel = CreateWidget<UInventoryPanel>(GetWorld(),InventoryPanelWidgetClass);
		InventoryPanel->AddToViewport();
		InventoryPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
	if(InteractionWidgetClass)
	{
		InteractionWidget = CreateWidget<UInteractionWidget>(GetWorld(),InteractionWidgetClass);
		InteractionWidget->AddToViewport(-1);
		InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	// if(MainMenuWidgetClass)
	// {
	// 	MainMenuWidget = CreateWidget<UMainMenuWidget>(GetWorld(),MainMenuWidgetClass);
	// 	MainMenuWidget->AddToViewport(5);
	// 	MainMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	// }
}

void AGameHUD::ShowInventory()
{
	if(InventoryPanel)
	{
		IsInventoryVisible = true;
		InventoryPanel->ShowInventory();
	}
}

void AGameHUD::HideInventory()
{
	if(InventoryPanel)
	{
		IsInventoryVisible = false;
		InventoryPanel->HideInventory();
	}
}

void AGameHUD::ShowInteractionWidget() const
{
	if(InteractionWidget)
	{
		InteractionWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AGameHUD::HideInteractionWidget() const
{
	if(InteractionWidget)
	{
		InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AGameHUD::UpdateInteractionWidget(const FInteractableData* InteractableData) const
{
	if(InteractionWidget)
	{
		if(InteractionWidget->GetVisibility() == ESlateVisibility::Collapsed)
		{
			InteractionWidget->SetVisibility(ESlateVisibility::Visible);
		}
		InteractionWidget->UpdateWidget(InteractableData);
	}
}
