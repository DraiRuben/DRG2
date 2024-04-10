// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/InteractionWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Items/Interfaces/Interactable.h"

void UInteractionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	InteractionProgressBar->PercentDelegate.BindUFunction(this,"UpdateInteractionProgress");
}

void UInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	KeyPressText->SetText(FText::FromString("Press"));
	CurrentInteractionDuration = 0.0f;
}
void UInteractionWidget::UpdateWidget(const FInteractableData* InteractableData) const
{
	switch(InteractableData->InteractableType) {
	case EInteractableType::Pickup:
		KeyPressText->SetText(FText::FromString("Press"));
		InteractionProgressBar->SetVisibility(ESlateVisibility::Collapsed);
		if(InteractableData->Quantity==1)
		{
			Quantity->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			Quantity->SetText(FText::Format(NSLOCTEXT("InteractionWidget","QuantityText","x{0}"),
				InteractableData->Quantity));
			Quantity->SetVisibility(ESlateVisibility::Visible);
		}
		
		break;
	case EInteractableType::NonPlayerCharacter:
		
		break;
	case EInteractableType::Device:
		
		break;
	case EInteractableType::Toggle:
		
		break;
	case EInteractableType::Container:
		
		break;
	}
	ActionText->SetText(InteractableData->Action);
	NameText->SetText(InteractableData->Name);
}

float UInteractionWidget::UpdateInteractionProgress()
{
	return 0.0f;
}