

#include "Items/Pickup.h"

#include "PlayerCharacter.h"
#include "Items/InventoryComponent.h"
#include "Items/ItemBase.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = false;
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMesh->SetSimulatePhysics(true);
	PickupMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PickupMesh->SetCollisionObjectType(ECC_PhysicsBody);
	PickupMesh->SetCollisionResponseToChannel(ECC_PhysicsBody,ECR_Ignore);
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
	SetRootComponent(PickupMesh);
}
void APickup::BeginPlay()
{
	Super::BeginPlay();
	InitializePickup(UItemBase::StaticClass(),ItemQuantity);
}
void APickup::InitializePickup(const TSubclassOf<UItemBase> BaseClass, const int32 Quantity)
{
	if(ItemDataTable && !DesiredItemID.IsNone())
	{
		const FItemData* ItemData = ItemDataTable->FindRow<FItemData>(DesiredItemID,DesiredItemID.ToString());

		ItemReference = NewObject<UItemBase>(this,BaseClass);
		ItemReference->ID = ItemData->ID;
		ItemReference->ItemType = ItemData->ItemType;
		ItemReference->ItemRarity = ItemData->ItemRarity;
		ItemReference->NumericData = ItemData->NumericData;
		ItemReference->AssetData = ItemData->AssetData;
		ItemReference->DescriptionData = ItemData->DescriptionData;

		Quantity <=0 ?ItemReference->SetQuantity(1) : ItemReference->SetQuantity(Quantity);

		PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);
		PickupMesh->SetMaterial(0,ItemData->AssetData.Material);
		UpdateInteractableData();
	}
}

void APickup::InitializeDrop(UItemBase* ItemToDrop, const int32 Quantity)
{
	ItemReference = ItemToDrop;
	Quantity <=0 ?ItemReference->SetQuantity(1) : ItemReference->SetQuantity(Quantity);
	PickupMesh->SetStaticMesh(ItemToDrop->AssetData.Mesh);
	PickupMesh->SetMaterial(0,ItemToDrop->AssetData.Material);

	UpdateInteractableData();
}

void APickup::BeginFocus()
{
	IInteractable::BeginFocus();
	if(PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(true);
	}
}

void APickup::EndFocus()
{
	IInteractable::EndFocus();
	if(PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(false);
	}
}

void APickup::Interact(APlayerCharacter* PlayerCharacter)
{
	if(PlayerCharacter)
	{
		TakePickup(PlayerCharacter);
	}
}

void APickup::UpdateInteractableData()
{
	InstanceInteractableData.InteractableType = EInteractableType::Pickup;
	InstanceInteractableData.Action = ItemReference->DescriptionData.InteractionText;
	InstanceInteractableData.Name = ItemReference->DescriptionData.Name;
	InstanceInteractableData.Quantity = ItemReference->Quantity;
	InteractableData = InstanceInteractableData;
	
}

void APickup::TakePickup(const APlayerCharacter* Taker)
{
	if(!IsPendingKillPending() && ItemReference)
	{
		if(UInventoryComponent* PlayerInventory = Taker->GetInventory())
		{
			const FItemAddResult AddResult = PlayerInventory->HandleAddItem(ItemReference);
			switch(AddResult.OperationResult) {
			case EItemAddResult::IAR_NoItemAdded:
				break;
			case EItemAddResult::IAR_PartialAmountAdded:
				UpdateInteractableData();
				Taker->UpdateInteractionWidget();
				break;
			case EItemAddResult::IAR_AllItemAdded:
				Destroy();
				break;
			}

			UE_LOG(LogTemp, Warning, TEXT("%s"),*AddResult.ResultMessage.ToString());
		
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Inventory Component Reference is null"));

		}
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup internal item reference is null"));

	}
}

void APickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName ChangedPropertyName = PropertyChangedEvent.Property?PropertyChangedEvent.Property->GetFName() : NAME_None;
	if(ChangedPropertyName == GET_MEMBER_NAME_CHECKED(APickup, DesiredItemID))
	{
		if(ItemDataTable)
		{
			const FString ContextString{DesiredItemID.ToString()};
			if(const FItemData* ItemData = ItemDataTable->FindRow<FItemData>(DesiredItemID, DesiredItemID.ToString()))
			{
				PickupMesh ->SetStaticMesh(ItemData->AssetData.Mesh);
				PickupMesh->SetMaterial(0, ItemData->AssetData.Material);
			}
		}
	}
}



