// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "Chunk.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
AChunkWorld::AChunkWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ChunkCount = 0;
}

void AChunkWorld::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
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
			if(Generate3D)
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
				FVector(X * Size.X * 100, Y * Size.Y * 100, Z* Size.Z),
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
	GeneratedChunks[GetChunkIndex(X,Y,Z)] = chunk;
}

bool AChunkWorld::IsChunkPosValid(const FIntVector ChunkPos) const
{
	if(ChunkPos.X<0 || ChunkPos.Y<0 || ChunkPos.Z<0)
	{
		return false;
	}

	if(ChunkPos.X>=DrawDistance.X || ChunkPos.Y>=DrawDistance.Y || ChunkPos.Z >=DrawDistance.Z || (ChunkPos.Z!=0 && !Generate3D))
	{
		return false;
	}
	return true;
}

int AChunkWorld::GetChunkIndex(const int X, const int Y, const int Z) const
{
	if(Generate3D)
		return Z  + Y*DrawDistance.Y+ X*DrawDistance.Y*DrawDistance.X;
	else
		return Y+ X*DrawDistance.X;
}

void AChunkWorld::TryGenerateNewChunks()
{
	bool Generated = false;
	for (int x = 0; x < DrawDistance.X; x++)
	{
		for (int y = 0; y < DrawDistance.Y; y++)
		{
			if(Generate3D)
			{
				
				for (int z = 0; z < DrawDistance.Z; z++)
				{
					if(x!= 0 && x!=DrawDistance.X-1
					&& y!=0 && y!= DrawDistance.Y-1
					&& z!= 0 && z!= DrawDistance.Z-1) continue;
					Generated = TryExpandMap(x,y,z)?true:Generated;
					
				}
				continue;
			}
			if(x!= 0 && x!=DrawDistance.X-1 && y!=0 && y!= DrawDistance.Y-1) continue;

			Generated = TryExpandMap(x,y,0)?true:Generated;
		}
	}
	if(Generated)
	{
		SetAdjacentChunks();


	}
}

bool AChunkWorld::TryExpandMap(const int X, const int Y, const int Z)
{
	const auto Player = UGameplayStatics::GetPlayerCharacter(GetWorld(),0);
	const auto PlayerPos = Player->GetActorLocation();
	if(GeneratedChunks[GetChunkIndex(X,Y,Z)]== nullptr) return false;
	
	const auto ChunkPos = GeneratedChunks[GetChunkIndex(X,Y,Z)]->GetActorLocation();
	if(FMath::Abs(ChunkPos.X-PlayerPos.X)<RenderDistance.X*100*Size.X
		||FMath::Abs(ChunkPos.Y-PlayerPos.Y)<RenderDistance.Y*100*Size.Y
		||FMath::Abs(ChunkPos.Z-PlayerPos.Z)<RenderDistance.Z*100*Size.Z)
	{

		int OffsetX = 0;
		int OffsetY = 0;
		int OffsetZ = 0;
		
		int StartX = 0;
		int StartY = 0;
		int StartZ = 0;
		
		int EndX = 0;
		int EndY = 0;
		int EndZ = 0;
		if(FMath::Abs(ChunkPos.X-PlayerPos.X)<RenderDistance.X*100*Size.X)
		{
			OffsetX = ChunkPos.X>PlayerPos.X?1:-1;
			StartX = X;
			EndX = X+1;
			StartY = 0;
			EndY = DrawDistance.Y;
			StartZ = 0;
			EndZ = DrawDistance.Z;
			DrawDistance.X++;
		}
		else if(FMath::Abs(ChunkPos.Y-PlayerPos.Y)<RenderDistance.Y*100*Size.Y)
		{
			OffsetY = ChunkPos.Y>PlayerPos.Y?1:-1;
			StartX = X;
			EndX = X+1;
			StartY=Y;
			EndY = Y+1;
			StartZ = 0;
			EndZ = DrawDistance.Z;
			DrawDistance.Y++;
		}
		else if(FMath::Abs(ChunkPos.Z-PlayerPos.Z)<RenderDistance.Z*100*Size.Z)
		{
			OffsetZ = ChunkPos.Z>PlayerPos.Z?1:-1;
			StartX = X;
			EndX = X+1;
			StartY=Y;
			EndY = Y+1;
			StartZ = Z;
			EndZ = DrawDistance.Z;
			DrawDistance.Z++;
		}
				
 		if(GeneratedChunks[GetChunkIndex(X+OffsetX,Y+OffsetY,Z+OffsetZ)]==nullptr)
		{
			MakeChunk(X+OffsetX,Y+OffsetY,Z+OffsetZ);
 			UE_LOG(LogTemp, Warning, TEXT("BEUTE"));
			return true;
		}
		for (int Xbis = StartX; Xbis < EndX; Xbis++)
		{
			for (int Ybis = StartY; Ybis < EndY; Ybis++)
			{
				for (int Zbis = StartZ; Zbis < EndZ; Zbis++)
				{
					GeneratedChunks.Insert(nullptr,GetChunkIndex(Xbis,Ybis,Zbis));
				}
			}
		}
			
		MakeChunk(X+OffsetX,Y+OffsetY,Z+OffsetZ);
		return true;
	}
	return false;
}


// Called when the game starts or when spawned
void AChunkWorld::BeginPlay()
{
	Super::BeginPlay();
	GeneratedChunks.SetNum(DrawDistance.X*DrawDistance.Y*DrawDistance.Z);
	GetWorldTimerManager().SetTimer(CheckTimer,this,&AChunkWorld::TryGenerateNewChunks,1.0f,true,7);
	if(Generate3D)
	{
		Generate3DMap();
	}else
	{
		Generate2DMap();
	}
	SetAdjacentChunks();
}

