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
	WaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>("WaterMesh");
	FastNoise = CreateDefaultSubobject<UFastNoiseWrapper>(TEXT("FastNoiseMaker"));

	SetRootComponent(Mesh);	
	Mesh->SetMobility(EComponentMobility::Static);

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
	const auto Dist = GenType == EGenerationType::Gen3D ? FVector::Distance(ChunkPos,PlayerPos):FVector::DistXY(ChunkPos,PlayerPos);
  	if(Dist<8000)
	{
		for (int i = 0; i < (GenType == EGenerationType::Gen3D?6:4); i++)
		{
			if(AdjacentChunks[i]==nullptr)
			{
				auto closestCell = Spawner->GetClosestChunkInDir(static_cast<EDirection>(i),ChunkPos);
				if(closestCell == nullptr)
				{
					const auto NewChunkPos = ChunkPos + static_cast<FVector>(AdjacentOffset[i]*Size*100.0f);
					AdjacentChunks[i] = Spawner->MakeChunk(NewChunkPos);
					for (int u = 0; u < (GenType == EGenerationType::Gen3D?6:4); u++)
					{
						AdjacentChunks[i]->AdjacentChunks[u] = Spawner->GetClosestChunkInDir(static_cast<EDirection>(u),NewChunkPos);
					}
				}
				else
				{
					AdjacentChunks[i] = closestCell;
				}
			}
		}
	}
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{

	WaterMesh->AttachToComponent(Mesh,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	WaterMesh->SetMobility(EComponentMobility::Static);
	Blocks.SetNum(Size.X * Size.Y * Size.Z);
	FastNoise->SetupFastNoise(EFastNoise_NoiseType::Cubic,Seed, Frequency,EFastNoise_Interp::Quintic,EFastNoise_FractalType::FBM,Octave,2,0,.45f,EFastNoise_CellularDistanceFunction::Euclidean,EFastNoise_CellularReturnType::CellValue);

	Mesh->bUseAsyncCooking = true;
	WaterMesh->bUseAsyncCooking = true;
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick([this]()	{
		for (int u = 0; u < (GenType == EGenerationType::Gen3D?6:4); u++)
		{
			if(AdjacentChunks[u]!=nullptr)
			{
				AdjacentChunks[u]->AdjacentChunks[GetInverseDirection(u)] = this;
			}
		}
	});
	
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,[&]()
	{
		const auto GenTask = new FAsyncTask<FAsyncChunkGenerator>(this);
		GenTask->StartBackgroundTask();
		GenTask->EnsureCompletion();
		delete GenTask;
		AsyncTask(ENamedThreads::GameThread,[this]()
		{
			ApplyMesh();
		});
		
	});
}

void AChunk::GenerateTrees(TArray<FIntVector> TrunkPositions)
{
	FRandomStream Stream = FRandomStream(GetUniqueID()+Seed);
	for(auto Trunk : TrunkPositions)
	{
		int TreeHeight = Stream.RandRange(3,6);
		int RandX = Stream.RandRange(0,2);
		int RandY = Stream.RandRange(0,2);
		int RandZ = Stream.RandRange(0,2);

		for(int treeX = -2;treeX<3;treeX++)
		{
			for (int treeY = -2; treeY < 3; treeY++)
			{
				for (int treeZ = -2; treeZ < 3; treeZ++)
				{
					if(treeX + Trunk.X>=0 && treeX + Trunk.X<Size.X
						&&treeY + Trunk.Y>=0 && treeY + Trunk.Y<Size.Y
						&&treeZ + Trunk.Z>=0 && treeZ + Trunk.Z<Size.Z)
					{
						float radius = FVector(treeX* RandX, treeY*RandY, treeZ*RandZ).Size();

						if(radius<=2.8f)
						{
							if(Stream.FRand()<0.5f || radius <=1.2f)
							{
								Blocks[GetBlockIndex(Trunk.X+ treeX,Trunk.Y+treeY,Trunk.Z+treeZ+ TreeHeight)] = EBlock::Leaves;
							}
						}
					}
				}
			}
		}
		for (int i = 0; i < TreeHeight; i++)
		{
			Blocks[GetBlockIndex(Trunk.X,Trunk.Y,Trunk.Z+ i)] = EBlock::Trunk;
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
						if (VoxelType == static_cast<int>(EBlock::Trunk)-2 || Check(GetPositionInDirection(Direction, Position)))
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
	switch(GenType)
	{
	case EGenerationType::Gen2D:
		GenerateHeightMap2D();
		break;
	case EGenerationType::Gen3D:
		GenerateHeightMap3D();
		break;
	case EGenerationType::GenComplete:
		GenerateCompleteMap();
		break;
	}
	GenerateMesh();
	ApplyMesh();
}

void AChunk::ApplyMesh()
{
	//TODO: Use UpdateMeshSection for modify voxel instead
	Mesh->ClearAllMeshSections();
	WaterMesh->ClearAllMeshSections();
	for (int i = 0; i < ChunkDataPerMat.Num()-1; ++i)
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
	if(MakeWater)
	{
		WaterMesh->CreateMeshSection(0,
				ChunkDataPerMat[5].VertexData,
				ChunkDataPerMat[5].TriangleData,
				ChunkDataPerMat[5].Normals,
				ChunkDataPerMat[5].UVData,
				ChunkDataPerMat[5].Colors,
				TArray<FProcMeshTangent>(),
				true);
		WaterMesh->SetMaterial(0,Materials[5]);
	}
	for (int i = 0; i < Materials.Num()-1; i++)
	{
		Mesh->SetMaterial(i,Materials[i]);
	}
	
}
	

void AChunk::GenerateHeightMap3D()
{
	const FVector Position = GetActorLocation()/100;
	for (int x = 0; x < Size.X; ++x)
	{
		for (int y = 0; y < Size.Y; ++y)
		{
			for (int z = 0; z < Size.Z; ++z)
			{
				const auto NoiseValue = FastNoise->GetNoise3D(x + Position.X, y + Position.Y, z + Position.Z);

				if (NoiseValue >= 0)
				{
					Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
				}
				else
				{
					Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
				}
			}
		}
	}
}

void AChunk::GenerateHeightMap2D()
{
	TArray<FIntVector> TrunkPositions;
	FRandomStream Stream = FRandomStream(GetUniqueID()+Seed);
	FRandomStream GlobalStream = FRandomStream(Seed);
	WaterLevel = GlobalStream.RandRange(MinWaterHeight,MaxWaterHeight);
	const auto Location = GetActorLocation();
	for (int x = 0; x < Size.X; ++x) 
	{
		const float Xpos = (x * 100 + Location.X) / 100;
		for (int y = 0; y < Size.Y; ++y) 
		{
			const float Ypos = (y * 100 + Location.Y) / 100;
			const int Height = FMath::Clamp(FMath::RoundToInt((FastNoise->GetNoise2D(Xpos,Ypos)+1)*Size.Z/24),0,Size.Z)+ZOffset;
			for (int z = 0; z < Size.Z; z++)
			{
				if(MakeWater)
				{
					if (z < Height - 3) Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
					else if (z < Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Dirt;
					else if (z == Height - 1) Blocks[GetBlockIndex(x, y, z)] = z>=WaterLevel?EBlock::Grass:EBlock::Dirt;
					else if(z == Height && z>WaterLevel && Stream.FRand()<0.01f)TrunkPositions.Add(FIntVector(x,y,z));
					else Blocks[GetBlockIndex(x, y, z)] = z<=WaterLevel?EBlock::Water:EBlock::Air;
					
				}
				else
				{
					if (z < Height - 3) Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
					else if (z < Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Dirt;
					else if (z == Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Grass;
					else if(z == Height && Stream.FRand()<0.01f)TrunkPositions.Add(FIntVector(x,y,z));
					else Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
				}
			}
		}
	}
	GenerateTrees(TrunkPositions);
}

void AChunk::GenerateCompleteMap()
{
	TArray<FIntVector> TrunkPositions;
	FRandomStream Stream = FRandomStream(GetUniqueID()+Seed);
	FRandomStream GlobalStream = FRandomStream(Seed);
	WaterLevel = GlobalStream.RandRange(MinWaterHeight,MaxWaterHeight);
	const FVector Location = GetActorLocation();
	const float CaveCoordMultiplier =CaveFrequency/Frequency;
	for (int x = 0; x < Size.X; ++x)
	{
		const float Xpos = (x * 100 + Location.X) / 100;
		for (int y = 0; y < Size.Y; ++y)
		{
			const float Ypos = (y * 100 + Location.Y) / 100;
			const int Height = FMath::Clamp(FMath::RoundToInt((FastNoise->GetNoise2D(Xpos,Ypos)+1)*Size.Z/24),0,Size.Z)+ZOffset;
			for (int z = 0; z < Size.Z; ++z)
			{
				const auto Noise3DValue = FastNoise->GetNoise3D((x + Location.X/100)*CaveCoordMultiplier, (y + Location.Y/100)*CaveCoordMultiplier, (z + Location.Z/100)*CaveCoordMultiplier);
				if(MakeWater)
				{
					if (z < Height - 3)
					{
						if (Noise3DValue + HeightNoiseAdjustment->GetFloatValue(
							static_cast<float>(z) / static_cast<float>(Height)) > CaveEmptyThreshold)
						{
							Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
						}
						else
						{
							Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
						}
					}
					else if (z < Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Dirt;
					else if (z == Height - 1) Blocks[GetBlockIndex(x, y, z)] = z>=WaterLevel?EBlock::Grass:EBlock::Dirt;
					else if(z == Height && z>WaterLevel && Stream.FRand()<0.01f)TrunkPositions.Add(FIntVector(x,y,z));
					else Blocks[GetBlockIndex(x, y, z)] =z<=WaterLevel?EBlock::Water: EBlock::Air;
				}
				else
				{
					if (z < Height - 3)
					{
						if (Noise3DValue + HeightNoiseAdjustment->GetFloatValue(
							static_cast<float>(z) / static_cast<float>(Height)) > CaveEmptyThreshold)
						{
							Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
						}
						else
						{
							Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
						}
					}
					else if (z < Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Dirt;
					else if (z == Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Grass;
					else if(z == Height && Stream.FRand()<0.01f)TrunkPositions.Add(FIntVector(x,y,z));
					else Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
				}
				
			}
		}
	}
	GenerateTrees(TrunkPositions);
}

bool AChunk::Check(FVector Position) const
{
	if (Position.X >= Size.X || Position.Y >= Size.Y || Position.Z >= Size.Z ||Position.X < 0 || Position.Y < 0 || Position.Z < 0) return true;

	return Blocks[GetBlockIndex(Position.X, Position.Y, Position.Z)] == EBlock::Air || Blocks[GetBlockIndex(Position.X, Position.Y, Position.Z)] == EBlock::Water;
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
	if (Position.X >= Size.X || Position.Y >= Size.Y || Position.Z >= Size.Z ||Position.X < 0 || Position.Y < 0 || Position.Z < 0)
	{
		return;
	}
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

void FAsyncChunkGenerator::DoWork()
{
	Chunk->ClearMesh();
	switch(Chunk->GenType)
	{
	case EGenerationType::Gen2D:
		Chunk->GenerateHeightMap2D();
		break;
	case EGenerationType::Gen3D:
		Chunk->GenerateHeightMap3D();
		break;
	case EGenerationType::GenComplete:
		Chunk->GenerateCompleteMap();
		break;
	}
	Chunk->GenerateMesh();
}
