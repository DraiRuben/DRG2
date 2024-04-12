// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InventoryComponent.h"

#include "Items/ItemBase.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}
// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}
UItemBase* UInventoryComponent::FindMatchingItem(UItemBase* ItemIn) const
{
	if(ItemIn)
	{
		if(InventoryContents.Contains(ItemIn))
		{
			return ItemIn;
		}
	}
	return nullptr;
}

UItemBase* UInventoryComponent::FindNextItemByID(UItemBase* ItemIn) const
{
	if(ItemIn)
	{
		if(const TArray<TObjectPtr<UItemBase>>::ElementType* Result = InventoryContents.FindByKey(ItemIn))
		{
			return *Result;
		}
	}
	return nullptr;
}

int32 UInventoryComponent::CalculateNumberForFullStack(UItemBase* StackableItem, int32 InitialRequestedItemAmount)
{
	const int32 AddAmountToMakeFullStack = StackableItem->NumericData.StackLimit - StackableItem->Quantity;
	return FMath::Min(InitialRequestedItemAmount, AddAmountToMakeFullStack);
}

UItemBase* UInventoryComponent::FindNextPartialStack(UItemBase* ItemIn) const
{
	if(const TArray<TObjectPtr<UItemBase>>::ElementType* Result
		= InventoryContents.FindByPredicate([&ItemIn](const UItemBase* InventoryItem)
	{
		return InventoryItem->ID == ItemIn->ID && !InventoryItem->IsFullItemStack();
	}
	))
	{
		return *Result;
	}
	return nullptr;
}

void UInventoryComponent::RemoveSingleInstanceOfItem(UItemBase* ItemIn)
{
	InventoryContents.RemoveSingle(ItemIn);
	OnInventoryUpdated.Broadcast();
}

int32 UInventoryComponent::RemoveAmountOfItem(UItemBase* ItemIn, int32 DesiredAmountToRemove)
{
	const int32 ActualAmountToRemove = FMath::Min(DesiredAmountToRemove, ItemIn->Quantity);
	ItemIn->SetQuantity(ItemIn->Quantity - ActualAmountToRemove);
	OnInventoryUpdated.Broadcast();
	return ActualAmountToRemove;
}

void UInventoryComponent::SplitExistingStack(UItemBase* ItemIn, const int32 AmountToSplit)
{
	if(!(InventoryContents.Num()+1>InventorySlotsCapacity))
	{
		RemoveAmountOfItem(ItemIn,AmountToSplit);
		AddNewItem(ItemIn,AmountToSplit);
	}
}

FItemAddResult UInventoryComponent::HandleNonStackableItems(UItemBase* InputItem, int32 RequestedAddAmount)
{
	if(InventoryContents.Num()+1 >InventorySlotsCapacity)
	{
		return FItemAddResult::AddedNone(
			FText::Format(FText::FromString("Could not add {0} to the inventory. All inventory slots are full."), InputItem->DescriptionData.Name));
	}
	AddNewItem(InputItem, 1);
	return FItemAddResult::AddedAll(
		1,
		FText::Format(FText::FromString("Successfully added a single {0} to the inventory."), InputItem->DescriptionData.Name));
}

int32 UInventoryComponent::HandleStackableItems(UItemBase* InputItem, int32 RequestedAddAmount)
{
	if(RequestedAddAmount<=0)
	{
		return 0;
	}
	int32 AmountToDistribute = RequestedAddAmount;
	UItemBase* ExistingItem = FindNextPartialStack(InputItem);
	//Distribute item stack over existing stacks
	while(ExistingItem)
	{
		const int32 AmountToMakeFullStack = CalculateNumberForFullStack(ExistingItem, AmountToDistribute);
		if(AmountToMakeFullStack>0)
		{
			ExistingItem->SetQuantity(ExistingItem->Quantity + AmountToMakeFullStack);

			AmountToDistribute-=AmountToMakeFullStack;
			InputItem->SetQuantity(AmountToDistribute);
		}
		else
		{
			if(AmountToDistribute!=RequestedAddAmount)
			{
				OnInventoryUpdated.Broadcast();
				return RequestedAddAmount - AmountToDistribute;
			}
			return 0;
		}
		if(AmountToDistribute<=0)
		{
			OnInventoryUpdated.Broadcast();
			return RequestedAddAmount;
		}
		ExistingItem = FindNextPartialStack(InputItem);
	}
	//try to add new stack
	if(InventoryContents.Num() +1 <=InventorySlotsCapacity)
	{
		if(AmountToDistribute>0)
		{
			InputItem->SetQuantity(AmountToDistribute);
			AddNewItem(InputItem->CreateItemCopy(),AmountToDistribute);
			return RequestedAddAmount;
		}
	}
	OnInventoryUpdated.Broadcast();
	return RequestedAddAmount - AmountToDistribute;
}


FItemAddResult UInventoryComponent::HandleAddItem(UItemBase* InputItem)
{
	if(GetOwner())
	{
		const int32 InitialRequestedAddAmount = InputItem->Quantity;

		if(!InputItem->NumericData.IsStackable)
		{
			return HandleNonStackableItems(InputItem, InitialRequestedAddAmount);
		}
		const int32 StackableAmountAdded = HandleStackableItems(InputItem, InitialRequestedAddAmount);

		if(StackableAmountAdded == InitialRequestedAddAmount)
		{
			return FItemAddResult::AddedAll(InitialRequestedAddAmount, FText::Format(
				FText::FromString("Successfully added {0} {1} to the inventory."),
				StackableAmountAdded,
				InputItem->DescriptionData.Name));
		}
		if(StackableAmountAdded <InitialRequestedAddAmount && StackableAmountAdded >0)
		{
			return FItemAddResult::AddedPartial(StackableAmountAdded, FText::Format(
				FText::FromString("Partial amount {0} added to the inventory. Number added = {1}"),
				InputItem->DescriptionData.Name,
				StackableAmountAdded));
		}
		if(StackableAmountAdded<0)
		{
			return FItemAddResult::AddedNone(FText::Format(
				FText::FromString("Couldn't add {0} to the inventory. No remaining slots or item invalid"),
				InputItem->DescriptionData.Name));
		}
	}
	return FItemAddResult::AddedNone(FText::FromString("TryAddItem GetOwner() check failed"));
}

void UInventoryComponent::AddNewItem(UItemBase* Item, const int32 AmountToAdd)
{
	UItemBase* NewItem;
	if(Item->IsCopy || Item->IsPickup)
	{
		NewItem = Item;
		NewItem->ResetItemFlags();
	}
	else
	{
		NewItem = Item->CreateItemCopy();
	}

	NewItem->OwningInventory = this;
	NewItem->SetQuantity(AmountToAdd);

	InventoryContents.Add(NewItem);
	OnInventoryUpdated.Broadcast();
}
