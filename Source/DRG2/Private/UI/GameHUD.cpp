// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameHUD.h"

AGameHUD::AGameHUD()
{
}

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();
	if(MainMenuWidgetClass)
	{
		MainUI = CreateWidget<UMainMenuWidget>(GetWorld(),MainMenuWidgetClass);
		MainUI->AddToViewport(5);
		MainUI->SetVisibility(ESlateVisibility::Visible);
	}
	if(InteractionWidgetClass)
	{
		InteractionWidget = CreateWidget<UInteractionWidget>(GetWorld(),InteractionWidgetClass);
		InteractionWidget->AddToViewport(-1);
		InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AGameHUD::ShowInventory()
{
	if(MainUI)
	{
		IsInventoryVisible = true;
		MainUI->InventoryWidget->ShowInventory();
		FInputModeGameAndUI InputMode;
		GetOwningPlayerController()->SetInputMode(InputMode);
		GetOwningPlayerController()->SetShowMouseCursor(true);
	}
}

void AGameHUD::HideInventory()
{
	if(MainUI)
	{
		IsInventoryVisible = false;
		MainUI->InventoryWidget->HideInventory();
		FInputModeGameOnly InputMode;
		GetOwningPlayerController()->SetInputMode(InputMode);
		GetOwningPlayerController()->SetShowMouseCursor(false);
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
