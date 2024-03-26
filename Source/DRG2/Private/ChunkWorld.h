// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkWorld.generated.h"

UCLASS()
class AChunkWorld : public AActor
{
	GENERATED_BODY()
	
private:
	void Generate2DMap();
	void Generate3DMap();
	int ChunkCount;	

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
	UPROPERTY(EditAnywhere, Category = "Chunk World")
	FIntVector Size = FIntVector(32,32,32);
	UPROPERTY(EditAnywhere, Category = "ChunkWorld")
	float Frequency=0.1f;
	UPROPERTY(EditAnywhere, Category = "ChunkWorld")
	float Octaves=3;
	UPROPERTY(EditAnywhere, Category = "ChunkWorld")
	int Seed = 1337;
	UPROPERTY(EditAnywhere, Category = "ChunkWorld")
	TObjectPtr<UMaterialInterface> Material;
	UPROPERTY(EditAnywhere, Category = "HeightMap")
	bool Generate3D =false;

};
