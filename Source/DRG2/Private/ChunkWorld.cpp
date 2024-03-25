// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "Chunk.h"
// Sets default values
AChunkWorld::AChunkWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AChunkWorld::BeginPlay()
{
	Super::BeginPlay();
	const AChunk* ChunkDefaultClass = Chunk->GetDefaultObject<AChunk>();
	ChunkSizeX = ChunkDefaultClass->SizeX;
	ChunkSizeY = ChunkDefaultClass->SizeY;
	ChunkSizeZ = ChunkDefaultClass->SizeZ;
	for (int x = 0; x < DrawDistance; ++x)
	{
		for (int y = 0; y < DrawDistance; ++y)
		{
			GetWorld()->SpawnActor<AActor>(Chunk,FVector(x*ChunkSizeX*100,y*ChunkSizeY*100,0),FRotator::ZeroRotator);
		}
	}
}

