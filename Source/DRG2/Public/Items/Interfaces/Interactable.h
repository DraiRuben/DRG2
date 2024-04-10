// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class APlayerCharacter;
UENUM(BlueprintType)
enum class EInteractableType : uint8
{
	Pickup UMETA(DisplayName = "Pickup"),
	NonPlayerCharacter UMETA(DisplayName = "NonPlayerCharacter"),
	Device UMETA(DisplayName = "Device"),
	Toggle UMETA(DisplayName = "Toggle"),
	Container UMETA(DisplayName = "Container")
};

USTRUCT(BlueprintType)
struct FInteractableData
{
	GENERATED_USTRUCT_BODY()
	FInteractableData(): InteractableType(EInteractableType::Pickup),
	Name(FText::GetEmpty()),
	Action(FText::GetEmpty()),
	Quantity(0),
	InteractionDuration(0.0f)
	{
		
	};
	UPROPERTY(EditInstanceOnly)
	EInteractableType InteractableType;
	UPROPERTY(EditInstanceOnly)
	FText Name;
	UPROPERTY(EditInstanceOnly)
	FText Action;
	UPROPERTY(EditInstanceOnly)
	int32 Quantity;
	UPROPERTY(EditInstanceOnly)
	float InteractionDuration;
};

USTRUCT(BlueprintType)
struct FInteractionData
{
	GENERATED_USTRUCT_BODY()
	FInteractionData():CurrentInteractable(nullptr),
	LastInteractionCheckTime(0.0f)
	{
		
	}
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	AActor* CurrentInteractable;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float LastInteractionCheckTime;
	
};
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DRG2_API IInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void BeginFocus();
	virtual void EndFocus();
	virtual void BeginInteract();
	virtual void EndInteract();
	virtual void Interact(APlayerCharacter* PlayerCharacter);

	FInteractableData InteractableData;
};
