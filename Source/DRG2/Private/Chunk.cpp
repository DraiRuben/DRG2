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

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Blocks.SetNum(SizeX * SizeY * SizeZ);
	FastNoise->SetupFastNoise(EFastNoise_NoiseType::Perlin,Seed, Frequency,EFastNoise_Interp::Quintic,EFastNoise_FractalType::FBM,Octave,2,0.5f,1.0f,EFastNoise_CellularDistanceFunction::Euclidean,EFastNoise_CellularReturnType::Distance);

	Super::BeginPlay();
	
	GenerateChunk();
}


void AChunk::GenerateBlocks()
{
	const auto Location = GetActorLocation();
	for (int x = 0; x < SizeX; ++x) 
	{
		for (int y = 0; y < SizeY; ++y) 
		{
			const float Xpos = (x * 100 + Location.X) / 100;
			const float Ypos = (y * 100 + Location.Y) / 100;
			const int Height = FMath::Clamp(FMath::RoundToInt((FastNoise->GetNoise2D(Xpos,Ypos)+1)*SizeZ/2),0,SizeZ);
			for (int z = 0; z < Height; z++)
			{
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
			}
			for (int z = Height; z < SizeZ; z++)
			{
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
			}
		}
	}
}

void AChunk::GenerateMesh()
{
	for (int x = 0; x < SizeX; x++)
	{
		for (int y = 0; y < SizeY; y++)
		{
			for (int z = 0; z < SizeZ; z++)
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

void AChunk::GenerateChunk()
{
	GenerateBlocks();
	GenerateMesh();
	ApplyMesh();
}

void AChunk::ApplyMesh()
{
	Mesh->CreateMeshSection(0,VertexData,TriangleData,TArray<FVector>(),UVData,TArray<FColor>(),TArray<FProcMeshTangent>(),true); 
}

bool AChunk::Check(FVector Position) const
{
	if (Position.X >= SizeX || Position.Y >= SizeY || Position.Z >= SizeZ ||Position.X < 0 || Position.Y < 0 || Position.Z < 0) return true;

	return Blocks[GetBlockIndex(Position.X, Position.Y, Position.Z)] == EBlock::Air;
}

void AChunk::CreateFace(EDirection Direction, FVector Position)
{
	const auto Color = FColor::MakeRandomColor();
	const auto Normal = GetNormal(Direction);

	VertexData.Append(GetFaceVertices(Direction, Position));
	TriangleData.Append({ VertexCount + 3, VertexCount + 2, VertexCount, VertexCount + 2, VertexCount + 1, VertexCount });
	
	UVData.Append({ FVector2D(1,1), FVector2D(1,0), FVector2D(0,0), FVector2D(0,1) });
	/*Normals.Append({ Normal, Normal, Normal, Normal });
	Colors.Append({ Color, Color, Color, Color });*/
	VertexCount += 4;
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
	return Z * SizeX * SizeY + Y * SizeX + X;
}
