// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkWorld.generated.h"

UCLASS()
class AChunkWorld : public AActor
{
	GENERATED_BODY()
	
public:	


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	// Sets default values for this actor's properties
	AChunkWorld();
	UPROPERTY(EditAnywhere, Category="Chunk World")
	TSubclassOf<AActor> Chunk;
	UPROPERTY(EditAnywhere, Category="Chunk World")
	int DrawDistance = 5;

	int ChunkSizeX = 32;
	int ChunkSizeY = 32;
	int ChunkSizeZ = 32;

};
