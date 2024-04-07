// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enums.h"
#include "GameFramework/Actor.h"
#include "ChunkWorld.generated.h"
class AChunk;

UCLASS()
class AChunkWorld : public AActor
{
	GENERATED_BODY()
	
private:
	void Generate2DMap();
	void Generate3DMap();
	void SetAdjacentChunks();
	bool IsChunkPosValid(const FIntVector ChunkPos) const;
	int GetChunkIndex(const int X,const int Y,const int Z) const;
	int ChunkCount;
	void TryGenerateNewChunks();
	TArray<AChunk*> GeneratedChunks;
	FTimerHandle CheckTimer;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	// Sets default values for this actor's properties
	AChunkWorld();
	void MakeChunk(const int X, const int Y, const int Z);
	AChunk* MakeChunk(FVector Pos);
	AChunk* GetClosestChunkInDir(EDirection Direction,FVector ChunkPos);
	UPROPERTY(EditAnywhere, Category="SpawnParameters",meta = (EditCondition = "GenType != EGenerationType::Gen3D",EditConditionHides))
	int ZOffset;
	UPROPERTY(EditAnywhere, Category="SpawnParameters")
	TSubclassOf<AActor> Chunk;
	UPROPERTY(EditAnywhere, Category="SpawnParameters")
	FIntVector DrawDistance;
	UPROPERTY(EditAnywhere, Category="SpawnParameters")
	FIntVector RenderDistance;
	UPROPERTY(EditAnywhere, Category="SpawnParameters")
	FIntVector ChunkSpawnOffset;
	UPROPERTY(EditAnywhere, Category = "Chunks")
	FIntVector Size = FIntVector(32,32,32);
	UPROPERTY(EditAnywhere, Category = "Chunks")
	float Frequency=0.1f;
	UPROPERTY(EditAnywhere, Category = "Chunks", meta = (EditCondition = "GenType == EGenerationType::GenComplete",EditConditionHides))
	float CaveFrequency= 0.03f;
	UPROPERTY(EditAnywhere, Category = "Chunks", meta = (EditCondition = "GenType == EGenerationType::GenComplete",EditConditionHides))
	float CaveEmptyThreshold = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Chunks",meta = (EditCondition = "GenType != EGenerationType::Gen3D",EditConditionHides))
	bool MakeWater = false;
	UPROPERTY(EditAnywhere, Category = "Chunks", meta = (EditCondition = "MakeWater",EditConditionHides, UIMin = 0.0f, UIMax = 384.0f))
	float MinWaterHeight = 1;
	UPROPERTY(EditAnywhere, Category = "Chunks", meta = (EditCondition = "MakeWater",EditConditionHides, UIMin = 0.0f, UIMax = 384.0f))
	float MaxWaterHeight = 1;
	
	UPROPERTY(EditAnywhere, Category = "Chunks")
	float Octaves=3;
	UPROPERTY(EditAnywhere, Category = "Chunks")
	int Seed = 1337;
	UPROPERTY(EditAnywhere, Category = "Chunks")
	TArray<UMaterialInterface*> Materials;
	UPROPERTY(EditAnywhere, Category = "Chunks")
	EGenerationType GenType;
};
static FIntVector AdjacentOffset[8] =
	{
	FIntVector(1,0,0),
	FIntVector(0,1,0),
	FIntVector(-1,0,0),
	FIntVector(0,-1,0),
	FIntVector(0,0,1),
	FIntVector(0,0,-1),
};

