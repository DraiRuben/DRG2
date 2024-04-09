// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "Chunks/BaseChunk/Chunk.h"
#include "Chunks/Data/Enums.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
AChunkWorld::AChunkWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ChunkCount = 0;
}

void AChunkWorld::Setup()
{
	GeneratedChunks.SetNum(DrawDistance.X*DrawDistance.Y*DrawDistance.Z);
	GetWorldTimerManager().SetTimer(CheckTimer,this,&AChunkWorld::TryGenerateNewChunks,1.0f,true,1);
	switch(GenType)
	{
	case EGenerationType::Gen2D:
		Generate2DMap();
		break;
	case EGenerationType::Gen3D:
		Generate3DMap();
		break;
	case EGenerationType::GenComplete:
		Generate3DMap();
		break;
	}
	SetAdjacentChunks();
}

void AChunkWorld::Generate2DMap()
{
	for (int x = 0; x < DrawDistance.X; x++)
	{
		for (int y = 0; y < DrawDistance.Y; y++)
		{
			MakeChunk(x,y,0);
			ChunkCount++;
			
		}
	}
}

void AChunkWorld::Generate3DMap()
{
	for (int x = 0; x < DrawDistance.X; x++)
	{
		for (int y = 0; y < DrawDistance.Y; y++)
		{
			for (int z = 0; z < DrawDistance.Z; z++)
			{
				MakeChunk(x,y,z);
				ChunkCount++;
			}
		}
	}
}

void AChunkWorld::SetAdjacentChunks()
{
	for (int x = 0; x < DrawDistance.X; x++)
	{
		for (int y = 0; y < DrawDistance.Y; y++)
		{
			if(GenType != EGenerationType::Gen2D)
			{
				for (int z = 0; z < DrawDistance.Z; z++)
				{
					for (int i = 0; i < 6 ; i++)
					{
						
						const auto OffsetChunkPos = FIntVector(x,y,z) + AdjacentOffset[i];
						if(IsChunkPosValid(OffsetChunkPos))
						{
							if(GeneratedChunks[GetChunkIndex(x,y,z)]!=nullptr)
							GeneratedChunks[GetChunkIndex(x,y,z)]->AdjacentChunks[i] =
								GeneratedChunks[GetChunkIndex(OffsetChunkPos.X,OffsetChunkPos.Y,OffsetChunkPos.Z)];
						}
					}
					
				}
				continue;
			}

			for (int i = 0; i < 4 ; i++)
			{				
				const auto OffsetChunkPos = FIntVector(x,y,0) + AdjacentOffset[i];
				if(IsChunkPosValid(OffsetChunkPos))
				{
					if(GeneratedChunks[GetChunkIndex(x,y,0)]!=nullptr)
					GeneratedChunks[GetChunkIndex(x,y,0)]->AdjacentChunks[i] =
						GeneratedChunks[GetChunkIndex(OffsetChunkPos.X,OffsetChunkPos.Y,0)];

				}
			}
		}
	}
}

void AChunkWorld::MakeChunk(const int X, const int Y, const int Z)
{
	auto transform = FTransform(
				FRotator::ZeroRotator,
				FVector(
					X * Size.X  * 100 + ChunkSpawnOffset.X *100 * Size.X,
					Y * Size.Y *100 + ChunkSpawnOffset.Y * 100* Size.Y ,
					Z* Size.Z *100 + ChunkSpawnOffset.Z*100 * Size.Z),
				FVector::OneVector
			);

	const auto chunk = GetWorld()->SpawnActorDeferred<AChunk>(
		Chunk,
		transform,
		this
	);
	
	chunk->GenType = GenType;
	chunk->Seed = Seed;
	chunk->Frequency = Frequency;
	chunk->CaveFrequency = CaveFrequency;
	chunk->Octave = Octaves;
	chunk->Size = Size;
	chunk->Materials = Materials;
	chunk->ZOffset = ZOffset;
	chunk->SpawnOffset = ChunkSpawnOffset;
	chunk->CaveEmptyThreshold = CaveEmptyThreshold;
	chunk->MakeWater = MakeWater;
	chunk->MinWaterHeight = MinWaterHeight;
	chunk->MaxWaterHeight = MaxWaterHeight;
	chunk->Spawner = this;
	GeneratedChunks[GetChunkIndex(X,Y,Z)] = chunk;
	UGameplayStatics::FinishSpawningActor(chunk, transform);
}

AChunk* AChunkWorld::MakeChunk(FVector Pos)
{
	auto transform = FTransform(
				FRotator::ZeroRotator,
				Pos,
				FVector::OneVector
			);

	const auto chunk = GetWorld()->SpawnActorDeferred<AChunk>(
		Chunk,
		transform,
		this
	);
	
	chunk->GenType = GenType;
	chunk->Seed = Seed;
	chunk->Frequency = Frequency;
	chunk->CaveFrequency = CaveFrequency;
	chunk->Octave = Octaves;
	chunk->Size = Size;
	chunk->Materials = Materials;
	chunk->SpawnOffset = ChunkSpawnOffset;
	chunk->ZOffset = ZOffset;
	chunk->CaveEmptyThreshold = CaveEmptyThreshold;
	chunk->MakeWater = MakeWater;
	chunk->MinWaterHeight = MinWaterHeight;
	chunk->MaxWaterHeight = MaxWaterHeight;
	chunk->Spawner = this;
	GeneratedChunks.Add(chunk);
	UGameplayStatics::FinishSpawningActor(chunk, transform);
	return chunk;
}

AChunk* AChunkWorld::GetClosestChunkInDir(EDirection Direction,FVector ChunkPos)
{
	switch(Direction)
	{
	case EDirection::Forward:
		for (auto chunkP : GeneratedChunks)
		{
			auto chunkpP = chunkP->GetActorLocation();
			if(ChunkPos.Z == chunkpP.Z && ChunkPos.Y == chunkpP.Y && ChunkPos.X<chunkpP.X && FMath::Abs(chunkpP.X-ChunkPos.X) < Size.X*100+5)
			{
				return chunkP;
			}
		}
		break;
	case EDirection::Right:
		for (auto chunkP : GeneratedChunks)
		{
			auto chunkpP = chunkP->GetActorLocation();
			if(ChunkPos.Z == chunkpP.Z && ChunkPos.X == chunkpP.X && ChunkPos.Y<chunkpP.Y && FMath::Abs(chunkpP.Y-ChunkPos.Y) < Size.Y*100+5)
			{
				return chunkP;
			}
		}
		break;
	case EDirection::Left:
		for (auto chunkP : GeneratedChunks)
		{
			auto chunkpP = chunkP->GetActorLocation();
			if(ChunkPos.Z == chunkpP.Z && ChunkPos.X == chunkpP.X && ChunkPos.Y>chunkpP.Y && FMath::Abs(ChunkPos.Y-chunkpP.Y) < Size.Y*100+5)
			{
				return chunkP;
			}
		}
		break;
	case EDirection::Back:
		for (auto chunkP : GeneratedChunks)
		{
			auto chunkpP = chunkP->GetActorLocation();
			if(ChunkPos.Z == chunkpP.Z && ChunkPos.Y == chunkpP.Y && ChunkPos.X>chunkpP.X && FMath::Abs(ChunkPos.X-chunkpP.X) < Size.X*100+5)
			{
				return chunkP;
			}
		}
		break;
	case EDirection::Down:
		for (auto chunkP : GeneratedChunks)
		{
			auto chunkpP = chunkP->GetActorLocation();
			if(ChunkPos.X == chunkpP.X && ChunkPos.Y == chunkpP.Y && ChunkPos.Z>chunkpP.Z && FMath::Abs(ChunkPos.Z-chunkpP.Z) < Size.Z*100+5)
			{
				return chunkP;
			}
		}
		break;
	case EDirection::Up:
		for (auto chunkP : GeneratedChunks)
		{
			auto chunkpP = chunkP->GetActorLocation();
			if(ChunkPos.X == chunkpP.X && ChunkPos.Y == chunkpP.Y && ChunkPos.Z<chunkpP.Z && FMath::Abs(chunkpP.Z-ChunkPos.Z)< Size.Z*100+5)
			{
				UE_LOG(LogTemp,Warning,TEXT("filled"));
				return chunkP;
			}
		}
		break;
	}
	
	return nullptr;
}

bool AChunkWorld::IsChunkPosValid(const FIntVector ChunkPos) const
{
	if(ChunkPos.X<0 || ChunkPos.Y<0 || ChunkPos.Z<0)
	{
		return false;
	}

	if(ChunkPos.X>=DrawDistance.X || ChunkPos.Y>=DrawDistance.Y || ChunkPos.Z >=DrawDistance.Z || (ChunkPos.Z!=0 && GenType == EGenerationType::Gen2D))
	{
		return false;
	}
	return true;
}

int AChunkWorld::GetChunkIndex(const int X, const int Y, const int Z) const
{
	return Z*DrawDistance.X *DrawDistance.Y + Y*DrawDistance.X+ X;
	
}

void AChunkWorld::TryGenerateNewChunks()
{
	bool Generated = false;
	const auto PlayerPos = UGameplayStatics::GetPlayerCharacter(GetWorld(),0)->GetActorLocation();
	for(int i =0;i<GeneratedChunks.Num();i++)
	{
		GeneratedChunks[i]->TryGenerateAdjacent(PlayerPos);
	}
}

// Called when the game starts or when spawned
void AChunkWorld::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimerForNextTick(this,&AChunkWorld::Setup);
}

