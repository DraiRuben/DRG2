// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ChunkWorld.h"
#include "GameModeMC.generated.h"

/**
 * 
 */
UCLASS()
class DRG2_API AGameModeMC : public AGameModeBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure,Category = "Persistence",meta = (DisplayName = "Get Chunk Generator",Keywords = "ChunkGenerator"))
	AChunkWorld* ChunkGenerator();
	UPROPERTY(EditAnywhere, NoClear, BlueprintReadWrite, Category=Classes)
	TSubclassOf<AChunkWorld> ChunkGeneratorClass;
protected:
	void BeginPlay() override;
private:
	UPROPERTY(Transient)
	AChunkWorld* ChunkGeneratorInstance;
};

