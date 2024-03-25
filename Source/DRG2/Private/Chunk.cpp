// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"

// Sets default values
AChunk::AChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChunk::GenerateBlocks()
{
}

void AChunk::GenerateMesh()
{
}

void AChunk::ApplyMesh()
{
}

bool AChunk::Check(FVector Position) const
{
	return false;
}

void AChunk::CreateFace(EDirection Direction, FVector Position)
{
}

TArray<FVector> AChunk::GetFaceVertices(EDirection Direction, FVector Position) const
{
	return TArray<FVector>();
}

FVector AChunk::GetPositionInDirection(EDirection Direction, FVector Position) const
{
	return FVector();
}

int AChunk::GetBlockIndex(int X, int Y, int Z) const
{
	return 0;
}
