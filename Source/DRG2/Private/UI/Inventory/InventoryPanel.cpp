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
		for (int i = 0; i < 8*6; i++)
		{
			const auto SlotWidget = CreateWidget<UInventoryItemSlot>(this,InventorySlotClass);
			const int Row = i/8;
			const int Column = i%8;
			if(i<8*5)
			{
				InventoryStorage->AddChildToUniformGrid(SlotWidget,Row,Column);
			}
			else
			{
				HotBar->AddChildToUniformGrid(SlotWidget,0,Column);
			}
		}
	}
}

bool UInventoryPanel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

int32 UInventoryPanel::GetEmptyCellPos() const
{
	TArray<UWidget*> HotbarChildren = InventoryStorage->GetAllChildren();
	for(int i =0;i<HotbarChildren.Num();i++)
	{
		auto CastedHotBarChild = Cast<UInventoryItemSlot>(HotbarChildren[i]);
		if(!CastedHotBarChild->GetItemReference())
		{
			return i;
		}
	}
	TArray<UWidget*> StorageChildren = InventoryStorage->GetAllChildren();
	for(int i =0;i<StorageChildren.Num();i++)
	{
		auto CastedStorageChild = Cast<UInventoryItemSlot>(StorageChildren[i]);
		if(!CastedStorageChild->GetItemReference())
		{
			return i+8;
		}
	}
	return -1;
}

UWidget* UInventoryPanel::GetSlotAtIndex(const int32 Index) const
{
	if(Index<8)
	{
		return HotBar->GetChildAt(Index);
	}
	else
	{
		return InventoryStorage->GetChildAt(Index-8);
	}
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
		auto InventoryContents = InventoryReference->GetInventoryContents();
		for(int i =0;i<8*6;i++)
		{
			UWidget* SlotWidget = GetSlotAtIndex(i);
            UInventoryItemSlot* CastedSlot = Cast<UInventoryItemSlot>(SlotWidget);
			CastedSlot->SetItemReference(nullptr);
			CastedSlot->UpdateSlot();
		}
		for (int i = 0; i < InventoryContents.Num(); i++)
		{
			UItemBase* Item = InventoryContents[i];
			UWidget* SlotWidget = GetSlotAtIndex(i);
			UInventoryItemSlot* CastedSlot = Cast<UInventoryItemSlot>(SlotWidget);
			if(CastedSlot->GetItemReference() && CastedSlot->GetItemReference() != Item)
			{
				const int32 NewCellPos = GetEmptyCellPos();
				if(NewCellPos>=0)
				{
					SlotWidget = GetSlotAtIndex(NewCellPos);
					CastedSlot = Cast<UInventoryItemSlot>(SlotWidget);
				}
			}
			CastedSlot->SetItemReference(Item);
			CastedSlot->UpdateSlot();
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
		return row*8+column;
	}
	return -1;

}
