// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "UI/GameHUD.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> objectType(TEXT("/Game/MapGeneration/Mats/Water/MPC_Water.MPC_Water"));
	if (objectType.Succeeded())
	{
		WaterCollection = objectType.Object;
	}
	if(GetWorld())
		WaterParams = GetWorld()->GetParameterCollectionInstance(WaterCollection);


}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	EIC->BindAction(JumpAction, ETriggerEvent::Started,this,&APlayerCharacter::JumpStart);
	EIC->BindAction(JumpAction, ETriggerEvent::Triggered,this,&APlayerCharacter::JumpTrigger);
	EIC->BindAction(AimAction, ETriggerEvent::Triggered,this,&APlayerCharacter::Aim);
	EIC->BindAction(MoveAction, ETriggerEvent::Triggered,this,&APlayerCharacter::Move);
	EIC->BindAction(SprintAction, ETriggerEvent::Started,this,&APlayerCharacter::SprintStart);
	EIC->BindAction(SprintAction, ETriggerEvent::Canceled,this,&APlayerCharacter::SprintStop);
	EIC->BindAction(SprintAction, ETriggerEvent::Completed,this,&APlayerCharacter::SprintStop);
	EIC->BindAction(AttackAction, ETriggerEvent::Started,this,&APlayerCharacter::TryAttack);
	EIC->BindAction(InteractAction, ETriggerEvent::Started,this,&APlayerCharacter::TryInteract);
}
// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	BaseEyeHeight=90.0f;
	HUD = Cast<AGameHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	float IsInWater =0;
	WaterParams->GetScalarParameterValue(TEXT("IsInWater"),IsInWater);
	GetCharacterMovement()->GetPhysicsVolume()->bWaterVolume = IsInWater>0.0f;

	if(GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime)>InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const auto InputValue = Value.Get<FVector2D>();
	FVector InputVector = FVector::ZeroVector;
	InputVector += InputValue.X * GetActorRightVector();
	InputVector += InputValue.Y * GetActorForwardVector();
	AddMovementInput(InputVector,1,true);
}

void APlayerCharacter::Aim(const FInputActionValue& Value)
{
	const auto InputValue = Value.Get<FVector2D>();
	AddControllerPitchInput(-1*InputValue.Y*AimSensitivity.Y);
	AddControllerYawInput(InputValue.X*AimSensitivity.X);
}

void APlayerCharacter::JumpTrigger(const FInputActionValue& Value)
{
	if(GetCharacterMovement()->IsSwimming())
		AddMovementInput(FVector(0,0,10000));

}

void APlayerCharacter::JumpStart(const FInputActionValue& Value)
{
	if(!GetCharacterMovement()->IsSwimming())
		Jump();
}

void APlayerCharacter::SprintStart(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void APlayerCharacter::SprintStop(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APlayerCharacter::TryInteract(const FInputActionValue& Value)
{
}

void APlayerCharacter::TryAttack(const FInputActionValue& Value)
{
}

void APlayerCharacter::PerformInteractionCheck()
{
	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();
	FVector TraceStart{GetPawnViewLocation()};
	FVector TraceEnd{TraceStart+(GetViewRotation().Vector()*InteractionCheckDistance)};
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	FHitResult TraceHit;
	if(GetWorld()->LineTraceSingleByChannel(TraceHit,TraceStart,TraceEnd,ECC_Visibility,QueryParams))
	{
		if(TraceHit.GetActor()->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
		{
			if(TraceHit.GetActor() != InteractionData.CurrentInteractable)
			{
				FoundInteractable(TraceHit.GetActor());
				return;
			}
			if(TraceHit.GetActor() == InteractionData.CurrentInteractable)
			{
				return;
			}
		}
	}
}

void APlayerCharacter::FoundInteractable(AActor* NewInteractable)
{
	if(IsInteracting())
	{
		EndInteract();
	}
	if(InteractionData.CurrentInteractable)
	{
		TargetInteractable = InteractionData.CurrentInteractable;
		TargetInteractable->EndFocus();
	}
	InteractionData.CurrentInteractable = NewInteractable;
	TargetInteractable = NewInteractable;

	HUD->UpdateInteractionWidget(&TargetInteractable->InteractableData);
	TargetInteractable->BeginFocus();
}

void APlayerCharacter::NotInteractableFound()
{
	if(IsInteracting())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);
	}
	if(InteractionData.CurrentInteractable)
	{
		if(IsValid(TargetInteractable.GetObject()))
		{
			TargetInteractable->EndFocus();
		}
		HUD->HideInteractionWidget();
		InteractionData.CurrentInteractable = nullptr;
		TargetInteractable = nullptr;
	}
}

void APlayerCharacter::BeginInteract()
{
	PerformInteractionCheck();
	if(InteractionData.CurrentInteractable)
	{
		if(IsValid(TargetInteractable.GetObject()))
		{
			TargetInteractable->BeginInteract();
			if(FMath::IsNearlyZero(TargetInteractable->InteractableData.InteractionDuration,0.1f))
			{
				Interact();
			}
			else
			{
				GetWorldTimerManager().SetTimer(
					TimerHandle_Interaction,
					this,
					&APlayerCharacter::Interact,
					TargetInteractable->InteractableData.InteractionDuration,
					false);
			}
		}
	}
}

void APlayerCharacter::EndInteract()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);
	if(IsValid(TargetInteractable.GetObject()))
	{
		TargetInteractable->EndInteract();
	}
}

void APlayerCharacter::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);
	if(IsValid(TargetInteractable.GetObject()))
	{
		TargetInteractable->Interact(this);
	}
}



