// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"

#include "ChunkWorld.h"
#include "Enums.h"
#include "FastNoiseWrapper.h"
#include "ProceduralMeshComponent.h"


// Sets default values
AChunk::AChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
	Mesh->SetCastShadow(false);

	FastNoise = CreateDefaultSubobject<UFastNoiseWrapper>("FastNoiseMaker");
	SetRootComponent(Mesh);
	AdjacentChunks.SetNum(8);
}

void AChunk::ModifyVoxel(const FIntVector Position, const EBlock Block, const float Radius, const bool Recursive)
{
	if (Position.X > Size.X || Position.Y > Size.Y || Position.Z > Size.Z ||Position.X < -1 || Position.Y < -1 || Position.Z < -1)
	{
		return;
	}
	RecursiveSetData = Recursive;
	ModifyVoxelData(Position, Block, Radius);
	ClearMesh();
	GenerateMesh();
	ApplyMesh();
}

void AChunk::TryGenerateAdjacent(const FVector PlayerPos)
{
	const auto ChunkPos = GetActorLocation();

  	if(FVector::Distance(ChunkPos,PlayerPos)<8000)
	{
		for (int i = 0; i < (Generate3D?6:4); i++)
		{
			if(AdjacentChunks[i]==nullptr)
			{
				const auto NewChunkPos = ChunkPos + static_cast<FVector>(AdjacentOffset[i]*Size*100.0f);
				AdjacentChunks[i] = Spawner->MakeChunk(NewChunkPos);
				for (int u = 0; u < (Generate3D?6:4); u++)
				{
					AdjacentChunks[i]->AdjacentChunks[u] = Spawner->GetClosestChunkInDir(static_cast<EDirection>(u),NewChunkPos);
				}
			}
		}
	}
	
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Blocks.SetNum(Size.X * Size.Y * Size.Z);
	FastNoise->SetupFastNoise(EFastNoise_NoiseType::Perlin,Seed, Frequency,EFastNoise_Interp::Quintic,EFastNoise_FractalType::FBM,Octave,2,0.5f,1.0f,EFastNoise_CellularDistanceFunction::Euclidean,EFastNoise_CellularReturnType::Distance);

	Super::BeginPlay();
	
	GenerateChunk();
}


void AChunk::GenerateBlocks()
{
	const auto Location = GetActorLocation();
	for (int x = 0; x < Size.X; ++x) 
	{
		for (int y = 0; y < Size.Y; ++y) 
		{
			const float Xpos = (x * 100 + Location.X) / 100;
			const float Ypos = (y * 100 + Location.Y) / 100;
			const int Height = FMath::Clamp(FMath::RoundToInt((FastNoise->GetNoise2D(Xpos,Ypos)+1)*Size.Z/2),0,Size.Z);
			for (int z = 0; z < Height; z++)
			{
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
			}
			for (int z = Height; z < Size.Z; z++)
			{
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
			}
		}
	}
}

void AChunk::GenerateMesh()
{
	for (int x = 0; x < Size.X; x++)
	{
		for (int y = 0; y < Size.Y; y++)
		{
			for (int z = 0; z < Size.Z; z++)
			{
				if (Blocks[GetBlockIndex(x, y, z)] != EBlock::Air)
				{
					const auto Position = FVector(x, y, z);
					int VoxelType = static_cast<int>(Blocks[GetBlockIndex(x,y,z)])-2;
					VoxelType = FMath::Clamp(VoxelType,0,Materials.Num()-1);
					for (auto Direction : { EDirection::Forward, EDirection::Right, EDirection::Back, EDirection::Left, EDirection::Up, EDirection::Down })
					{
						if (Check(GetPositionInDirection(Direction, Position)))
						{
							CreateFace(Direction, Position * 100, VoxelType);
						}
					}
				}
			}
		}
	}
}

void AChunk::ClearMesh()
{
	VertexCount =0;
	ChunkDataPerMat.Empty();
	VertexCountPerMat.Empty();
	ChunkDataPerMat.SetNum(Materials.Num());
	VertexCountPerMat.SetNum(Materials.Num());
}

void AChunk::GenerateChunk()
{
	ClearMesh();
	GenerateBlocks();
	if(Generate3D)
		GenerateHeightMap3D();
	else
		GenerateHeightMap2D();
	GenerateMesh();
	ApplyMesh();
}

void AChunk::ApplyMesh()
{
	Mesh->ClearAllMeshSections();
	for (int i = 0; i < ChunkDataPerMat.Num(); ++i)
	{
		if(ChunkDataPerMat[i].VertexData.Num()>0)
		{
			Mesh->CreateMeshSection(i,
				ChunkDataPerMat[i].VertexData,
				ChunkDataPerMat[i].TriangleData,
				ChunkDataPerMat[i].Normals,
				ChunkDataPerMat[i].UVData,
				ChunkDataPerMat[i].Colors,
				TArray<FProcMeshTangent>(),
				true);
		}
	}
	for (int i = 0; i < Materials.Num(); ++i)
	{
		Mesh->SetMaterial(i,Materials[i]);
	}
	
}
	

void AChunk::GenerateHeightMap3D()
{
}

void AChunk::GenerateHeightMap2D()
{
}

bool AChunk::Check(FVector Position) const
{
	if (Position.X >= Size.X || Position.Y >= Size.Y || Position.Z >= Size.Z ||Position.X < 0 || Position.Y < 0 || Position.Z < 0) return true;

	return Blocks[GetBlockIndex(Position.X, Position.Y, Position.Z)] == EBlock::Air;
}

void AChunk::CreateFace(EDirection Direction, FVector Position, const int MeshMat)
{
	const auto Color = FColor(255,255,255, static_cast<int>(Direction));
	const auto Normal = GetNormal(Direction);

	ChunkDataPerMat[MeshMat].VertexData.Append(GetFaceVertices(Direction, Position));
	ChunkDataPerMat[MeshMat].TriangleData.Append({
		VertexCountPerMat[MeshMat] + 3,
		VertexCountPerMat[MeshMat] + 2,
		VertexCountPerMat[MeshMat],
		VertexCountPerMat[MeshMat] + 2,
		VertexCountPerMat[MeshMat] + 1,
		VertexCountPerMat[MeshMat] });
	
	ChunkDataPerMat[MeshMat].UVData.Append({
		FVector2D(1,0),
		FVector2D(0,0),
		FVector2D(0,1),
		FVector2D(1,1) });
	ChunkDataPerMat[MeshMat].Normals.Append({ Normal, Normal, Normal, Normal });
	ChunkDataPerMat[MeshMat].Colors.Append({ Color, Color, Color, Color });
	VertexCountPerMat[MeshMat] +=4;
}

void AChunk::ModifyVoxelData(const FIntVector Position, EBlock Block, const float Radius)
{
	if(RecursiveSetData)
	{
		for (int i = 0; i < 6; i++)
		{
			const FIntVector OtherLocalChunkPos = Position + AdjacentOffset[i] * Size*-1 ;
			if(AdjacentChunks[i] != nullptr)
			{
				UE_LOG(LogTemp,Warning,TEXT("%s"),*AdjacentChunks[i]->GetName());
				AdjacentChunks[i]->ModifyVoxel(OtherLocalChunkPos,Block,Radius,false);
			}
			else
			{
				UE_LOG(LogTemp,Warning,TEXT("Null ptr at index %d"),i);
			}
		}
	}
	const int Index = GetBlockIndex(Position.X,Position.Y,Position.Z);
	if(Index >=0)
		Blocks[Index] = Block;
}

TArray<FVector> AChunk::GetFaceVertices(EDirection Direction, FVector Position) const
{
	TArray<FVector> Vertices;
	for (int i = 0; i < 4; i++)
	{
		Vertices.Add(BlockVertexData[BlockTriangleData[i + static_cast<int>(Direction) * 4]] + Position);
	}

	return Vertices;
}

FVector AChunk::GetPositionInDirection(EDirection Direction, FVector Position) const
{
	switch (Direction)
	{
		case EDirection::Forward: return Position + FVector::ForwardVector;
		case EDirection::Back: return Position + FVector::BackwardVector;
		case EDirection::Left: return Position + FVector::LeftVector;
		case EDirection::Right: return Position + FVector::RightVector;
		case EDirection::Up: return Position + FVector::UpVector;
		case EDirection::Down: return Position + FVector::DownVector;
		default: throw std::invalid_argument("Invalid direction");
	}
}

FVector AChunk::GetNormal(const EDirection Direction) const
{
	switch (Direction)
	{
		case EDirection::Forward: return FVector::ForwardVector;
		case EDirection::Right: return FVector::RightVector;
		case EDirection::Back: return FVector::BackwardVector;
		case EDirection::Left: return FVector::LeftVector;
		case EDirection::Up: return FVector::UpVector;
		case EDirection::Down: return FVector::DownVector;
		default: throw std::invalid_argument("Invalid direction");
	}
}

int AChunk::GetBlockIndex(int X, int Y, int Z) const
{
	return Z * Size.X * Size.Y + Y * Size.X + X;
}
