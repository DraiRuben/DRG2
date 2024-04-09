#include "MarchingChunk.h"

#include "Chunks/Generator/ChunkWorld.h"
#include "FastNoiseWrapper.h"
#include "Chunks/Data/Enums.h"

void AMarchingChunk::ModifyVoxel(const FIntVector Position, const EBlock Block, const float Radius, const bool Recursive)
{
	if (Position.X -Radius >= Size.X || Position.Y -Radius>= Size.Y|| Position.Z -Radius >= Size.Z 
		|| Position.X + Radius < 0  || Position.Y+ Radius < 0 || Position.Z + Radius< 0)
	{
		UE_LOG(LogTemp,Warning,TEXT("Returned"));
		return;
	}
	RecursiveSetData = Recursive;
	ModifyVoxelData(Position, Block, Radius);
	ClearMesh();
	GenerateMesh();
	ApplyMesh();
}

void AMarchingChunk::GenerateHeightMap3D()
{
	const auto Position = GetActorLocation()/100;
	for (int x = 0; x <= Size.X; ++x)
	{
		for (int y = 0; y <= Size.Y; ++y)
		{
			for (int z = 0; z <= Size.Z; ++z)
			{
				Voxels[GetVoxelIndex(x,y,z)] = FastNoise->GetNoise3D(x + Position.X, y + Position.Y, z + Position.Z);	
			}
		}
	}
}

void AMarchingChunk::GenerateHeightMap2D()
{
	const auto Position = GetActorLocation()/100;
	for (int x = 0; x <= Size.X; x++)
	{
		for (int y = 0; y <= Size.Y; y++)
		{
			const float Xpos = x + Position.X;
			const float ypos = y + Position.Y;
			
			const int Height = FMath::Clamp(FMath::RoundToInt((FastNoise->GetNoise2D(Xpos, ypos) + 1) * Size.Z / 2), 0, Size.Z);

			for (int z = 0; z < Height; z++)
			{
				Voxels[GetVoxelIndex(x,y,z)] = 1.0f;
			}

			for (int z = Height; z < Size.Z; z++)
			{
				Voxels[GetVoxelIndex(x,y,z)] = -1.0f;
			}
		}
	}
}

void AMarchingChunk::GenerateMesh()
{
	if (SurfaceLevel > 0.0f)
	{
		TriangleOrder[0] = 0;
		TriangleOrder[1] = 1;
		TriangleOrder[2] = 2;
	}
	else
	{
		TriangleOrder[0] = 2;
		TriangleOrder[1] = 1;
		TriangleOrder[2] = 0;
	}

	float Cube[8];
	
	for (int X = 0; X < Size.X; ++X)
	{
		for (int Y = 0; Y < Size.Y; ++Y)
		{
			for (int Z = 0; Z < Size.Z; ++Z)
			{
				for (int i = 0; i < 8; ++i)
				{
					Cube[i] = Voxels[GetVoxelIndex(X + VertexOffset[i][0],Y + VertexOffset[i][1],Z + VertexOffset[i][2])];
				}

				March(X,Y,Z, Cube);
			}
		}
	}
}

void AMarchingChunk::BeginPlay()
{
	Voxels.SetNum((Size.X + 1) * (Size.Y + 1) * (Size.Z + 1));
	Super::BeginPlay();
}

void AMarchingChunk::ModifyVoxelData(const FIntVector Position, EBlock Block, const float Radius)
{

	if(RecursiveSetData)
	{
		for (int i = 0; i < 6; i++)
		{
			const FIntVector OtherLocalChunkPos = Position + AdjacentOffset[i] * Size * -1;
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

	const FVector SphereCenter = (FVector)(Position);
	// Iterate through each voxel in the grid
	for (int x = 0; x <= Size.X; ++x)
	{
		for (int y = 0; y <= Size.Y; ++y)
		{
			for (int z = 0; z <= Size.Z; ++z)
			{
				const auto VoxelPos = FVector(x, y, z);
				const float Distance = FVector::Distance(VoxelPos,SphereCenter);

				if (Distance <= Radius)
				{
					Voxels[GetVoxelIndex(x, y, z)] = Block == EBlock::Air?0.0f:1.0f;
				}
			}
		}
	}
}

void AMarchingChunk::March(int X, int Y, int Z, const float Cube[8])
{
	int VertexMask = 0;
	FVector EdgeVertex[12];

	//Find which vertices are inside of the surface and which are outside
	for (int i = 0; i < 8; ++i)
	{
		if (Cube[i] <= SurfaceLevel) VertexMask |= 1 << i;
	}

	const int EdgeMask = CubeEdgeFlags[VertexMask];
	
	if (EdgeMask == 0) return;
		
	// Find intersection points
	for (int i = 0; i < 12; ++i)
	{
		if ((EdgeMask & 1 << i) != 0)
		{
			const float Offset = Interpolation ? GetInterpolationOffset(Cube[EdgeConnection[i][0]], Cube[EdgeConnection[i][1]]) : 0.5f;

			EdgeVertex[i].X = X + (VertexOffset[EdgeConnection[i][0]][0] + Offset * EdgeDirection[i][0]);
			EdgeVertex[i].Y = Y + (VertexOffset[EdgeConnection[i][0]][1] + Offset * EdgeDirection[i][1]);
			EdgeVertex[i].Z = Z + (VertexOffset[EdgeConnection[i][0]][2] + Offset * EdgeDirection[i][2]);
		}
	}
	for (int i = 0; i < 5; ++i)
	{
		if(TriangleConnectionTable[VertexMask][3*i]<0) break;
		auto V1 = EdgeVertex[TriangleConnectionTable[VertexMask][3 * i]] * 100;
		auto V2 = EdgeVertex[TriangleConnectionTable[VertexMask][3 * i + 1]] * 100;
		auto V3 = EdgeVertex[TriangleConnectionTable[VertexMask][3 * i + 2]] * 100;
		auto Normal = FVector::CrossProduct(V2-V1, V3-V1);
		Normal.Normalize();

		ChunkDataPerMat[0].VertexData.Append({V1, V2, V3});

		ChunkDataPerMat[0].TriangleData.Append({
			VertexCount + TriangleOrder[0],
			VertexCount + TriangleOrder[1],
			VertexCount + TriangleOrder[2]
		});

		ChunkDataPerMat[0].Normals.Append({
			Normal,
			Normal,
			Normal
		});
		const auto Color = FColor::MakeRandomColor();

		ChunkDataPerMat[0].Colors.Append({ Color, Color, Color});

		VertexCount+=3;
	}
	
}

int AMarchingChunk::GetVoxelIndex(int X, int Y, int Z) const
{
	return Z* (Size.X+1)*(Size.Y+1) + Y*(Size.X+1) + X;
}

float AMarchingChunk::GetInterpolationOffset(float V1, float V2) const
{
	const float Delta = V2-V1;
	return Delta == 0.0f?SurfaceLevel:(SurfaceLevel-V1)/Delta;
}
