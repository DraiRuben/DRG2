// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Pickup.generated.h"

class UItemBase;

UCLASS()
class DRG2_API APickup : public AActor, public IInteractable
{
private:
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	APickup();
	UPROPERTY(VisibleAnywhere, Category = "Pickup | Components")
	UStaticMeshComponent* PickupMesh;
	UPROPERTY(EditInstanceOnly, Category = "Pickup | Item Initialization")
	UDataTable* ItemDataTable;
	UPROPERTY(EditInstanceOnly, Category = "Pickup | Item Initialization")
	FName DesiredItemID;
	UPROPERTY(VisibleAnywhere, Category = "Pickup | Item Reference")
	UItemBase* ItemReference;
	UPROPERTY(EditInstanceOnly, Category = "Pickup | Item Initialization")
	int32 ItemQuantity;
	UPROPERTY(VisibleInstanceOnly, Category = "Pickup | Interaction")
	FInteractableData InstanceInteractableData;

	void InitializePickup(const TSubclassOf<UItemBase> BaseClass, const int32 Quantity);
	void InitializeDrop(UItemBase* ItemToDrop, const int32 Quantity);

	FORCEINLINE UItemBase* GetItemData() const {return ItemReference;};
	
	virtual void BeginInRange(APlayerCharacter* ClosePlayer) override;
	virtual void EndInRange(APlayerCharacter* FarPlayer) override;
protected:
	// Called when the game starts or when spawned
	APlayerCharacter* ClosestPlayer;
	float TimeSinceInRangeEntered;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void UpdateInteractableData();
	UFUNCTION()
	void TakePickup(const APlayerCharacter* Taker);
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
};
