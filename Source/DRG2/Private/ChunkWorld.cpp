// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "Chunk.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
AChunkWorld::AChunkWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
}

void AChunkWorld::Generate2DMap()
{
	for (int x = -DrawDistance; x <= DrawDistance; x++)
	{
		for (int y = -DrawDistance; y <= DrawDistance; ++y)
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
			chunk->Material = Material;
			UGameplayStatics::FinishSpawningActor(chunk, transform);

			ChunkCount++;
		}
	}
}

void AChunkWorld::Generate3DMap()
{
	for (int x = -DrawDistance; x <= DrawDistance; x++)
	{
		for (int y = -DrawDistance; y <= DrawDistance; ++y)
		{
			for (int z = -DrawDistance; z <= DrawDistance; ++z)
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
				chunk->Material = Material;
				UGameplayStatics::FinishSpawningActor(chunk, transform);

				ChunkCount++;
			}
		}
	}
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
}

