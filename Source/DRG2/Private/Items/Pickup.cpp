

#include "Items/Pickup.h"

#include "PlayerCharacter.h"
#include "Items/InventoryComponent.h"
#include "Items/ItemBase.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
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
	Tags.Add(FName("Pickup"));

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
	ItemReference->OwningInventory = nullptr;
	UpdateInteractableData();
}

void APickup::BeginInRange(APlayerCharacter* ClosePlayer)
{
	IInteractable::BeginInRange(ClosePlayer);
	const float DistanceToCurrentClosest = ClosestPlayer? FVector::Dist(ClosestPlayer->GetActorLocation(),GetActorLocation()):999999;
	const float DistanceToNewPlayer = FVector::Dist(ClosePlayer->GetActorLocation(),GetActorLocation());

	if(DistanceToNewPlayer<DistanceToCurrentClosest)
	{
		ClosestPlayer = ClosePlayer;
		TimeSinceInRangeEntered = 0.0f;
	}
}

void APickup::EndInRange(APlayerCharacter* FarPlayer)
{
	IInteractable::EndInRange(FarPlayer);
	if(ClosestPlayer == FarPlayer)
	{
		ClosestPlayer = nullptr;
		TimeSinceInRangeEntered = 0.0f;
		PickupMesh->SetEnableGravity(true);
	}
}
void APickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(ClosestPlayer)
	{
		TimeSinceInRangeEntered+=GetWorld()->DeltaTimeSeconds;
		if(TimeSinceInRangeEntered>0.5f)
		{
			PickupMesh->SetEnableGravity(false);
			const FVector PickupToPlayerVector = ClosestPlayer->GetActorLocation()-GetActorLocation();
			const float Distance = PickupToPlayerVector.Size();
			PickupMesh->AddForce(100*PickupToPlayerVector,NAME_None,true);
			if(Distance<=50)
			{
				TakePickup(ClosestPlayer);
			}
		}
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



