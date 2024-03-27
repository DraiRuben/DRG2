// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"
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
}

void AChunk::ModifyVoxel(const FIntVector Position, const EBlock Block, const float Radius, const bool Recursive)
{
	if (Position.X >= Size.X || Position.Y >= Size.Y || Position.Z >= Size.Z ||Position.X < 0 || Position.Y < 0 || Position.Z < 0) return;
	ModifyVoxelData(Position, Block, Radius);
	ClearMesh();
	GenerateMesh();
	ApplyMesh();
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

					for (auto Direction : { EDirection::Forward, EDirection::Right, EDirection::Back, EDirection::Left, EDirection::Up, EDirection::Down })
					{
						if (Check(GetPositionInDirection(Direction, Position)))
						{
							CreateFace(Direction, Position * 100);
						}
					}
				}
			}
		}
	}
}

void AChunk::ClearMesh()
{
	VertexCount = 0;
	ChunkData.Clear();
}

void AChunk::GenerateChunk()
{
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
	Mesh->SetMaterial(0,Material);
	Mesh->CreateMeshSection(0,
		ChunkData.VertexData,
		ChunkData.TriangleData,
		ChunkData.Normals,
		ChunkData.UVData,
		ChunkData.Colors,
		TArray<FProcMeshTangent>(),
		true); 
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

void AChunk::CreateFace(EDirection Direction, FVector Position)
{
	const auto Color = FColor::MakeRandomColor();
	const auto Normal = GetNormal(Direction);

	ChunkData.VertexData.Append(GetFaceVertices(Direction, Position));
	ChunkData.TriangleData.Append({ VertexCount + 3, VertexCount + 2, VertexCount, VertexCount + 2, VertexCount + 1, VertexCount });
	
	ChunkData.UVData.Append({ FVector2D(1,1), FVector2D(1,0), FVector2D(0,0), FVector2D(0,1) });
	ChunkData.Normals.Append({ Normal, Normal, Normal, Normal });
	ChunkData.Colors.Append({ Color, Color, Color, Color });
	VertexCount += 4;
}

void AChunk::ModifyVoxelData(const FIntVector Position, EBlock Block, const float Radius)
{
	const int Index = GetBlockIndex(Position.X,Position.Y,Position.Z);
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
