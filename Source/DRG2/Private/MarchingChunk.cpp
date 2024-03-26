// Fill out your copyright notice in the Description page of Project Settings.


#include "MarchingChunk.h"

#include "FastNoiseWrapper.h"

void AMarchingChunk::GenerateHeightMap()
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

		ChunkData.VertexData.Append({V1, V2, V3});

		ChunkData.TriangleData.Append({
			VertexCount + TriangleOrder[0],
			VertexCount + TriangleOrder[1],
			VertexCount + TriangleOrder[2]
		});

		ChunkData.Normals.Append({
			Normal,
			Normal,
			Normal
		});
		const auto Color = FColor::MakeRandomColor();

		ChunkData.Colors.Append({ Color, Color, Color});

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
