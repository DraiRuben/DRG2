// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"
#include "Chunks/Data/Enums.h"
#include "Chunks/Generator/ChunkWorld.h"
#include "FastNoiseWrapper.h"
#include "GameMode/GameModeMC.h"
#include "ProceduralMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialParameterCollection.h"


// Sets default values
AChunk::AChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
	const auto GM = Cast<AGameModeMC>(UGameplayStatics::GetGameMode(GetWorld()));
	if(IsValid(GM) && GM->ChunkGenerator()->MakeWater)
	{
		WaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>("WaterMesh");
		WaterMesh->OnComponentBeginOverlap.AddDynamic(this,&AChunk::OnWaterBeginOverlap);
		WaterMesh->OnComponentEndOverlap.AddDynamic(this,&AChunk::OnWaterEndOverlap);
		static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> objectType(TEXT("/Game/MapGeneration/Mats/Water/MPC_Water.MPC_Water"));
		if (objectType.Succeeded())
		{
			WaterCollection = objectType.Object;
		}
		WaterParams = GetWorld()->GetParameterCollectionInstance(WaterCollection);
	}

	FastNoise = CreateDefaultSubobject<UFastNoiseWrapper>(TEXT("FastNoiseMaker"));

	SetRootComponent(Mesh);	
	Mesh->SetMobility(EComponentMobility::Static);

	AdjacentChunks.SetNum(8);

}
// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	if(MakeWater)
	{
		WaterMesh->AttachToComponent(Mesh,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		WaterMesh->SetMobility(EComponentMobility::Static);
		WaterMesh->bUseAsyncCooking = true;
	}
	Blocks.SetNum(Size.X * Size.Y * Size.Z);
	FastNoise->SetupFastNoise(EFastNoise_NoiseType::Cubic,Seed, Frequency,EFastNoise_Interp::Quintic,EFastNoise_FractalType::FBM,Octave,2,0,.45f,EFastNoise_CellularDistanceFunction::Euclidean,EFastNoise_CellularReturnType::CellValue);
	Mesh->bUseAsyncCooking = true;
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
void AChunk::ModifyVoxel(const FIntVector Position, const EBlock Block, const float Radius, const bool Recursive)
{
	if (Position.X > Size.X || Position.Y > Size.Y || Position.Z > Size.Z ||Position.X < -1 || Position.Y < -1 || Position.Z < -1)
	{
		return;
	}
	RecursiveSetData = Recursive;
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,[this,Position,Block,Radius]
	{
		ModifyVoxelData(Position, Block, Radius);
	});
}

void AChunk::OnWaterBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor->ActorHasTag("Player"))
	{
		UE_LOG(LogTemp,Warning,TEXT("TEST"));
		WaterParams->SetScalarParameterValue(TEXT("Water Level"),WaterLevel*100+100);
		float IsInWater =0;
		WaterParams->GetScalarParameterValue(TEXT("IsInWater"),IsInWater);
		WaterParams->SetScalarParameterValue(TEXT("IsInWater"),IsInWater+1);
		GEngine->AddOnScreenDebugMessage (-1, 5.f, FColor::Red, FString::Printf(TEXT("Ta mère")));

	}
}

void AChunk::OnWaterEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if(OtherActor->ActorHasTag("Player"))
	{
		UE_LOG(LogTemp,Warning,TEXT("TEST"));
		float IsInWater =0;
		WaterParams->GetScalarParameterValue(TEXT("IsInWater"),IsInWater);
		WaterParams->SetScalarParameterValue(TEXT("IsInWater"),IsInWater-1);
		GEngine->AddOnScreenDebugMessage (-1, 5.f, FColor::Blue, FString::Printf(TEXT("La pute")));

	}
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

void AChunk::GenerateTrees(TArray<FIntVector> TrunkPositions)
{
	FRandomStream Stream = FRandomStream(GetUniqueID()+Seed);
	for(const auto Trunk : TrunkPositions)
	{
		const int TreeHeight = Stream.RandRange(3,6);
		const int RandX = Stream.RandRange(0,2);
		const int RandY = Stream.RandRange(0,2);
		const int RandZ = Stream.RandRange(0,2);

		for(int TreeX = -2;TreeX<3;TreeX++)
		{
			for (int TreeY = -2; TreeY < 3; TreeY++)
			{
				for (int TreeZ = -2; TreeZ < 3; TreeZ++)
				{
					if(TreeX + Trunk.X>=0 && TreeX + Trunk.X<Size.X
						&&TreeY + Trunk.Y>=0 && TreeY + Trunk.Y<Size.Y
						&&TreeZ + Trunk.Z>=0 && TreeZ + Trunk.Z<Size.Z)
					{
						const float Radius = FVector(TreeX* RandX, TreeY*RandY, TreeZ*RandZ).Size();

						if(Radius<=2.8f)
						{
							if(Stream.FRand()<0.5f || Radius <=1.2f)
							{
								Blocks[GetBlockIndex(Trunk.X+ TreeX,Trunk.Y+TreeY,Trunk.Z+TreeZ+ TreeHeight)] = EBlock::Leaves;
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

void AChunk::GenerateMinerals(TArray<FIntVector> OreOrigins)
{
	FRandomStream Stream = FRandomStream(GetUniqueID()+Seed+1);
	for(const auto OreOrigin:OreOrigins)
	{
		const int RandX = Stream.RandRange(0,2);
		const int RandY = Stream.RandRange(0,2);
		const int RandZ = Stream.RandRange(0,2);
		const EBlock ChosenOre = static_cast<EBlock>(Stream.RandRange(7,10));
		for(int OreX = -2;OreX<3;OreX++)
		{
			for (int OreY = -2; OreY < 3; OreY++)
			{
				for (int OreZ = -2; OreZ < 3; OreZ++)
				{
					if(OreX + OreOrigin.X>=0 && OreX + OreOrigin.X<Size.X
						&&OreY + OreOrigin.Y>=0 && OreY + OreOrigin.Y<Size.Y
						&&OreZ + OreOrigin.Z>=0 && OreZ + OreOrigin.Z<Size.Z)
					{
						const float Radius = FVector(OreX* RandX, OreY*RandY, OreZ*RandZ).Size();

						if(Radius<=2.8f)
						{
							const int BlockIndex = GetBlockIndex(OreOrigin.X+ OreX,OreOrigin.Y+OreY,OreOrigin.Z+OreZ);
							if((Stream.FRand()<0.5f || Radius <=1.2f) && Blocks[BlockIndex] == EBlock::Stone)
							{
								Blocks[BlockIndex] = ChosenOre;
							}
						}
					}
				}
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
	if(WaterMesh)
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
		constexpr int WaterIndex = static_cast<int>(EBlock::Water)-2;
		WaterMesh->CreateMeshSection(0,
				ChunkDataPerMat[WaterIndex].VertexData,
				ChunkDataPerMat[WaterIndex].TriangleData,
				ChunkDataPerMat[WaterIndex].Normals,
				ChunkDataPerMat[WaterIndex].UVData,
				ChunkDataPerMat[WaterIndex].Colors,
				TArray<FProcMeshTangent>(),
				true);
		WaterMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		WaterMesh->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
		WaterMesh->SetMaterial(0,Materials[WaterIndex]);
		WaterMesh->ClearCollisionConvexMeshes();
		WaterMesh->bUseComplexAsSimpleCollision = false;
		WaterMesh->AddCollisionConvexMesh(ChunkDataPerMat[WaterIndex].VertexData);
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
			const int Height = FMath::Clamp(FMath::RoundToInt((FastNoise->GetNoise2D(Xpos,Ypos)+1)*Amplitude),0,Size.Z)+ZOffset;
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
	TArray<FIntVector> OrePositions;
	FRandomStream Stream = FRandomStream(GetUniqueID()+Seed);
	FRandomStream GlobalStream = FRandomStream(Seed);
	WaterLevel = GlobalStream.RandRange(MinWaterHeight,MaxWaterHeight);
	const FVector Location = GetActorLocation();
	const float CaveCoordMultiplier =CaveFrequency/Frequency;
	int SplitZOffset =SplitChunkVertically?static_cast<int>(Location.Z/(100*Size.Z))-ZOffset:0;
	for (int x = 0; x < Size.X; ++x)
	{
		const float Xpos = (x * 100 + Location.X) / 100;
		for (int y = 0; y < Size.Y; ++y)
		{
			const float Ypos = (y * 100 + Location.Y) / 100;
			const int Height = FMath::Clamp(FMath::RoundToInt(
				(FastNoise->GetNoise2D(Xpos,Ypos)+1)*Amplitude),
				0,
				Size.Z)
			+ZOffset;
			
			for (int z = 0; z < Size.Z; ++z)
			{
				const auto Noise3DValue = FastNoise->GetNoise3D((x + Location.X/100)*CaveCoordMultiplier, (y + Location.Y/100)*CaveCoordMultiplier, (z + Location.Z/100)*CaveCoordMultiplier);
				if(MakeWater)
				{
					if (z + SplitZOffset < Height - 3)
					{
						if (Noise3DValue + HeightNoiseAdjustment->GetFloatValue(
							static_cast<float>(z) / static_cast<float>(Height)) > CaveEmptyThreshold)
						{
							Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
						}
						else
						{
							if(Stream.FRand()<0.0005f)
							{
								OrePositions.Add(FIntVector(x,y,z));
							}
							else
							{
								Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
							}
						}
					}
					else if (z + SplitZOffset < Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Dirt;
					else if (z + SplitZOffset == Height - 1) Blocks[GetBlockIndex(x, y, z)] = z>=WaterLevel?EBlock::Grass:EBlock::Dirt;
					else if(z + SplitZOffset == Height && z>WaterLevel && Stream.FRand()<0.01f)TrunkPositions.Add(FIntVector(x,y,z));
					else Blocks[GetBlockIndex(x, y, z)] =z<=WaterLevel?EBlock::Water: EBlock::Air;
				}
				else
				{
					if (z + SplitZOffset < Height - 3)
					{
						if (Noise3DValue + HeightNoiseAdjustment->GetFloatValue(
							static_cast<float>(z) / static_cast<float>(Height)) > CaveEmptyThreshold)
						{
							Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
						}
						else
						{
							if(Stream.FRand()<0.0005f)
							{
								OrePositions.Add(FIntVector(x,y,z));
							}
							else
							{
								Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
							}
						}
					}
					else if (z + SplitZOffset < Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Dirt;
					else if (z + SplitZOffset == Height - 1) Blocks[GetBlockIndex(x, y, z)] = EBlock::Grass;
					else if(z + SplitZOffset == Height && Stream.FRand()<0.01f)TrunkPositions.Add(FIntVector(x,y,z));
					else Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
				}
				
			}
		}
	}
	GenerateTrees(TrunkPositions);
	GenerateMinerals(OrePositions);
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
	ClearMesh();
	GenerateMesh();
	AsyncTask(ENamedThreads::GameThread,[this]()
	{
		ApplyMesh();
	});
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
