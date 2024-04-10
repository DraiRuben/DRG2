// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenuWidget.h"

#include "PlayerCharacter.h"

void UMainMenuWidget::NativeOnInitialized()
{
	UUserWidget::NativeOnInitialized();
}

void UMainMenuWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();
	PlayerCharacter = Cast<APlayerCharacter>(GetOwningPlayer());
}

bool UMainMenuWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	return UUserWidget::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
