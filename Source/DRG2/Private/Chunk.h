// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk.generated.h"

enum class EBlock;
enum class EDirection;
class UFastNoiseWrapper;
class UProceduralMeshComponent;

UCLASS()
class AChunk : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AChunk();
	UPROPERTY(EditAnywhere, Category = "Chunk")
	int SizeX = 32;
	UPROPERTY(EditAnywhere, Category = "Chunk")
	int SizeY = 32;
	UPROPERTY(EditAnywhere, Category = "Chunk")
	int SizeZ = 64;
	
	UPROPERTY(EditAnywhere, Category = "Chunk")
	float Frequency = 0.03f;

	UPROPERTY(EditAnywhere, Category = "Chunk")
	int Seed = 1337;

	UPROPERTY(EditAnywhere, Category = "Chunk")
	int Octave = 3;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TObjectPtr<UProceduralMeshComponent> Mesh;
	UPROPERTY(VisibleDefaultsOnly, Category = "Fast Noise")
	TObjectPtr<UFastNoiseWrapper> FastNoise;
	TArray<EBlock> Blocks;
	TArray<FVector>VertexData;
	TArray<int> TriangleData;
	TArray<FVector2D> UVData;
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

	void GenerateBlocks();
	void GenerateMesh();
	UFUNCTION(BlueprintCallable)
	void GenerateChunk();
	void ApplyMesh();
	bool Check(FVector Position) const;
	void CreateFace(EDirection Direction, FVector Position);
	TArray<FVector> GetFaceVertices(EDirection Direction, FVector Position) const;
	FVector GetPositionInDirection(EDirection Direction, FVector Position) const;
	FVector GetNormal(const EDirection Direction) const;
	int GetBlockIndex(int X, int Y, int Z) const; 
};
