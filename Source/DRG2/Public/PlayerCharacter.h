// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Items/Interfaces/Interactable.h"
#include "PlayerCharacter.generated.h"

class UItemBase;
class UInventoryComponent;
class AGameHUD;

UCLASS()
class DRG2_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();
	bool IsInteracting() const {return GetWorldTimerManager().IsTimerActive(TimerHandle_Interaction);}

	FORCEINLINE UInventoryComponent* GetInventory() const {return PlayerInventory;}

	void UpdateInteractionWidget() const;
	void DropItem(UItemBase* ItemToDrop, int32 QuantityToDrop);
public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Enhanced Input")
	UInputMappingContext* IMC;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* AimAction;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* SprintAction;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* InteractAction;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* AttackAction;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* InventoryAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Movement")
	float RunSpeed;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Movement")
	float WalkSpeed;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Movement")
	FVector2D AimSensitivity;
	
private:
	UMaterialParameterCollection* WaterCollection;
	UMaterialParameterCollectionInstance* WaterParams;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void Move(const FInputActionValue& Value);
	void Aim(const FInputActionValue& Value);
	void JumpTrigger(const FInputActionValue& Value);
	void JumpStart(const FInputActionValue& Value);
	void SprintStart(const FInputActionValue& Value);
	void SprintStop(const FInputActionValue& Value);
	void TryInteract(const FInputActionValue& Value);
	void TryAttack(const FInputActionValue& Value);
	void ToggleInventory(const FInputActionValue& Value);

	void PerformInteractionCheck();
	void FoundInteractable(AActor* NewInteractable);
	void NotInteractableFound();
	void BeginInteract();
	void EndInteract();
	void Interact();
protected:
	FInteractionData InteractionData;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float InteractionCheckFrequency;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float InteractionCheckDistance;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FTimerHandle TimerHandle_Interaction;
	UPROPERTY(VisibleAnywhere,Category = "Character | Interaction")
	TScriptInterface<IInteractable> TargetInteractable;

	UPROPERTY(VisibleAnywhere, Category = "Character | Inventory")
	UInventoryComponent* PlayerInventory;
	AGameHUD* HUD;

	bool Interacted;
};
