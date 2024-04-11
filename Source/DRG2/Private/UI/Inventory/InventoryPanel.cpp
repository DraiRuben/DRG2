// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/InventoryPanel.h"

#include "PlayerCharacter.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Items/InventoryComponent.h"
#include "Items/ItemBase.h"
#include "UI/Inventory/InventoryItemSlot.h"

void UInventoryPanel::NativeConstruct()
{
	Super::NativeConstruct();
	InventoryStorage->GetParent()->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	PlayerCharacter = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	if(PlayerCharacter)
	{
		InventoryReference = PlayerCharacter->GetInventory();
		if(InventoryReference)
		{
			InventoryReference->OnInventoryUpdated.AddUObject(this, &UInventoryPanel::RefreshInventory);
		}
	}
}

bool UInventoryPanel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
void UInventoryPanel::HideInventory()
{
	InventoryStorage->GetParent()->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryPanel::ShowInventory()
{
	InventoryStorage->GetParent()->SetVisibility(ESlateVisibility::Visible);
}
void UInventoryPanel::RefreshInventory()
{
	if(InventoryReference && InventorySlotClass)
	{
		InventoryStorage->ClearChildren();
		HotBar->ClearChildren();
		auto InventoryContents = InventoryReference->GetInventoryContents();
		for (int i = 0; i < InventoryContents.Num(); i++)
		{
			UInventoryItemSlot* ItemSlot = CreateWidget<UInventoryItemSlot>(this,InventorySlotClass);
			ItemSlot->SetItemReference(InventoryContents[i]);
			if(i<9*3)
			{
				int x = i/9;
				int y = i%9;
				InventoryStorage->AddChildToUniformGrid(ItemSlot,x,y);
			}
			else
			{
				int x = 0;
				int y = i%9;
				HotBar->AddChildToUniformGrid(ItemSlot,x,y);
			}
		}
	}
}

int32 UInventoryPanel::GetMouseDropPositionAsInventorySlotIndex()
{
	const auto Geometry = GetCachedGeometry();

	FVector2D pixelPosition;
	FVector2D viewportMinPosition;
	FVector2D viewportMaxPosition;

	// Get the top left and bottom right viewport positions
	USlateBlueprintLibrary::LocalToViewport(this,Geometry, FVector2D( 0, 0 ), pixelPosition, viewportMinPosition );
	USlateBlueprintLibrary::LocalToViewport(this,Geometry, Geometry.GetLocalSize(), pixelPosition, viewportMaxPosition );
	FVector2D mousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	FVector2D OriginOffset{0,Geometry.GetLocalSize().Y};
	FVector2D Position = mousePosition - viewportMinPosition;
	Position*= FVector2D(0,-1);
	Position+=OriginOffset;
	FVector2D NormalizedPosition = ( mousePosition - viewportMinPosition ) / ( viewportMaxPosition - viewportMinPosition );
	
	if(FMath::Min( NormalizedPosition.X, NormalizedPosition.Y ) >= 0.f
		&& FMath::Max( NormalizedPosition.X, NormalizedPosition.Y ) <= 1.f)
	{
		int row = Position.Y/100;
		int column = Position.X/100;
		return row+column*8;
	}
	return -1;

}
