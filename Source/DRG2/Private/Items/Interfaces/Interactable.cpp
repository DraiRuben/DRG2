// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Interfaces/Interactable.h"

// Add default functionality here for any IInteractable functions that are not pure virtual.
void IInteractable::BeginFocus()
{
}

void IInteractable::EndFocus()
{
}

void IInteractable::BeginInteract()
{
}

void IInteractable::EndInteract()
{
}

void IInteractable::BeginInRange(APlayerCharacter* ClosePlayer)
{
}


void IInteractable::EndInRange(APlayerCharacter* FarPlayer)
{
}

void IInteractable::Interact(APlayerCharacter* PlayerCharacter)
{
}
