﻿#pragma once

#include "CoreMinimal.h"
#include "ChunkMeshData.generated.h"

USTRUCT()
struct FChunkData
{
	GENERATED_BODY();
public:
	TArray<FVector>VertexData;
	TArray<int> TriangleData;
	TArray<FVector> Normals;
	TArray<FVector2D> UVData;
	TArray<FColor> Colors;
	void Clear();
};
inline void FChunkData::Clear()
{
	VertexData.Empty();
	TriangleData.Empty();
	Normals.Empty();
	UVData.Empty();
	Colors.Empty();
}