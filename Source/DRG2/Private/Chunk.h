// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkMeshData.h"
#include "ChunkWorld.h"
#include "Chunk.generated.h"


enum class EBlock : uint8;
enum class EDirection : uint8;
enum class EGenerationType:uint8;
class UFastNoiseWrapper;
class UProceduralMeshComponent;

UCLASS()
class AChunk : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AChunk();
	TObjectPtr<AChunkWorld> Spawner;
	FIntVector Size;
	float Frequency = 0.03f;
	int Seed = 1337;
	int Octave = 3;
	int ZOffset;
	
	UPROPERTY(EditDefaultsOnly, Category = "Chunk")
	UCurveFloat* HeightNoiseAdjustment;
	UPROPERTY(EditAnywhere, Category = "Chunk")
	EGenerationType GenType;
	UPROPERTY(EditAnywhere, Category = "Fast Noise")
	TArray<TObjectPtr<AChunk>> AdjacentChunks;
	UFUNCTION(BlueprintCallable, Category = "Chunk")
	virtual void ModifyVoxel(const FIntVector Position, const EBlock Block, const float Radius, const bool Recursive);
	
	bool RecursiveSetData = false;
	TArray<UMaterialInterface*> Materials;
	TArray<FChunkData>ChunkDataPerMat;
	TArray<int> VertexCountPerMat;
	FIntVector SpawnOffset;
	
	void TryGenerateAdjacent(const FVector PlayerPos);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere,Category = "Component")
	TObjectPtr<UProceduralMeshComponent> Mesh;
	UPROPERTY(VisibleDefaultsOnly, Category = "Fast Noise")
	TObjectPtr<UFastNoiseWrapper> FastNoise;
	TArray<EBlock> Blocks;
	int VertexCount = 0;
	const FVector BlockVertexData[8] = {
		FVector(100,100,100),
		FVector(100,0,100),
		FVector(100,0,0),
		FVector(100,100,0),
		FVector(0,0,100),
		FVector(0,100,100),
		FVector(0,100,0),
		FVector(0,0,0)
	};
	const int BlockTriangleData[24] = {
		0,1,2,3,
		5,0,3,6,
		4,5,6,7,
		1,4,7,2,
		5,4,1,0,
		3,2,7,6
	};
	UFUNCTION(BlueprintCallable)
	void ClearMesh();
	virtual void GenerateChunk();
	virtual void GenerateTrees(TArray<FIntVector> TrunkPositions);
	virtual void GenerateMesh();
	virtual void ApplyMesh();
	virtual void GenerateHeightMap3D();
	virtual void GenerateHeightMap2D();
	virtual void GenerateCompleteMap();
	virtual bool Check(FVector Position) const;
	virtual void CreateFace(EDirection Direction, FVector Position, const int MeshMat);
	virtual void ModifyVoxelData(const FIntVector Position, EBlock Block, const float Radius);
	virtual TArray<FVector> GetFaceVertices(EDirection Direction, FVector Position) const;
	virtual FVector GetPositionInDirection(EDirection Direction, FVector Position) const;
	virtual FVector GetNormal(const EDirection Direction) const;
	virtual int GetBlockIndex(int X, int Y, int Z) const; 
};
