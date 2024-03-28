// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "Chunk.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
AChunkWorld::AChunkWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	ChunkCount = 0;
}

void AChunkWorld::Generate2DMap()
{
	for (int x = -DrawDistance; x <= DrawDistance; x++)
	{
		for (int y = -DrawDistance; y <= DrawDistance; y++)
		{
			auto transform = FTransform(
				FRotator::ZeroRotator,
				FVector(x * Size.X * 100, y * Size.Y * 100, 0),
				FVector::OneVector
			);

			const auto chunk = GetWorld()->SpawnActorDeferred<AChunk>(
				Chunk,
				transform,
				this
			);
	
			chunk->Generate3D = Generate3D;
			chunk->Seed = Seed;
			chunk->Frequency = Frequency;
			chunk->Octave = Octaves;
			chunk->Size = Size;
			chunk->Materials = Materials;
			UGameplayStatics::FinishSpawningActor(chunk, transform);
			GeneratedChunks.Add(chunk);
			ChunkCount++;
			
		}
	}
}

void AChunkWorld::Generate3DMap()
{
	for (int x = -DrawDistance; x <= DrawDistance; x++)
	{
		for (int y = -DrawDistance; y <= DrawDistance; y++)
		{
			for (int z = -DrawDistance; z <= DrawDistance; z++)
			{
				auto transform = FTransform(
					FRotator::ZeroRotator,
					FVector(x * Size.X * 100, y * Size.Y * 100, z * Size.Z * 100),
					FVector::OneVector
				);
				
				const auto chunk = GetWorld()->SpawnActorDeferred<AChunk>(
					Chunk,
					transform,
					this
				);

				chunk->Generate3D = Generate3D;
				chunk->Seed = Seed;
				chunk->Frequency = Frequency;
				chunk->Octave = Octaves;
				chunk->Size = Size;
				chunk->Materials = Materials;
				UGameplayStatics::FinishSpawningActor(chunk, transform);
				GeneratedChunks.Add(chunk);
				ChunkCount++;
			}
		}
	}
}

void AChunkWorld::SetAdjacentChunks()
{
	for (int x = -DrawDistance; x <= DrawDistance; x++)
	{
		for (int y = -DrawDistance; y <= DrawDistance; y++)
		{
			if(Generate3D)
			{
				for (int z = -DrawDistance; z <= DrawDistance; z++)
				{
					for (int i = 0; i < 6 ; i++)
					{
						
						const auto OffsetChunkPos = FIntVector(x+DrawDistance,y+DrawDistance,z+DrawDistance) + AdjacentOffset[i];
						if(IsChunkPosValid(OffsetChunkPos))
						{
							GeneratedChunks[GetChunkIndex(x+DrawDistance,y+DrawDistance,z+DrawDistance)]->AdjacentChunks[i] =
								GeneratedChunks[GetChunkIndex(OffsetChunkPos.X,OffsetChunkPos.Y,OffsetChunkPos.Z)];
						}
					}
					
				}
				continue;
			}

			for (int i = 0; i < 4 ; i++)
			{				
				const auto OffsetChunkPos = FIntVector(x+DrawDistance,y+DrawDistance,0) + AdjacentOffset[i];
				if(IsChunkPosValid(OffsetChunkPos))
				{
					GeneratedChunks[GetChunkIndex(x+DrawDistance,y+DrawDistance,0)]->AdjacentChunks[i] =
						GeneratedChunks[GetChunkIndex(OffsetChunkPos.X,OffsetChunkPos.Y,0)];

				}
			}
		}
	}
}

bool AChunkWorld::IsChunkPosValid(const FIntVector ChunkPos) const
{
	if(ChunkPos.X<0 || ChunkPos.Y<0 || ChunkPos.Z<0)
	{
		return false;
	}

	if(ChunkPos.X>DrawDistance*2 || ChunkPos.Y>DrawDistance*2 || ChunkPos.Z >DrawDistance*2 || (ChunkPos.Z!=0 && !Generate3D))
	{
		return false;
	}
	return true;
}

int AChunkWorld::GetChunkIndex(const int X, const int Y, const int Z) const
{
	if(Generate3D)
		return Z  + Y*(DrawDistance*2+1)+ X*(DrawDistance*2+1)*(DrawDistance*2+1);
	else
		return Y+ X*(DrawDistance*2+1);
}

// Called when the game starts or when spawned
void AChunkWorld::BeginPlay()
{
	Super::BeginPlay();

	if(Generate3D)
	{
		Generate3DMap();
	}else
	{
		Generate2DMap();
	}
	SetAdjacentChunks();
}

