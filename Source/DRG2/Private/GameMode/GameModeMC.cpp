// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModeMC.h"

AChunkWorld* AGameModeMC::ChunkGenerator()
{
	if(!IsValid(ChunkGeneratorInstance))
	{
		auto world = GetWorld();
		if(IsValid(world))
		{
			ChunkGeneratorInstance = world->SpawnActor<AChunkWorld>(ChunkGeneratorClass,FVector(0,0,0), FRotator::ZeroRotator);
		}
	}
	return ChunkGeneratorInstance;
}

void AGameModeMC::BeginPlay()
{	
	Super::BeginPlay();
	ChunkGenerator();
}
