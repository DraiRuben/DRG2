#pragma once

#include "Chunk.h"
#include "OptimisedChunk.generated.h"
UCLASS()
class AOptimisedChunk: public AChunk
{
	struct FMask
	{
		EBlock Block;
		int Normal;
	};
public:
	GENERATED_BODY()
	AOptimisedChunk();
protected:
	virtual void BeginPlay() override;
	virtual void GenerateMesh() override;
	virtual void GenerateHeightMap2D() override;
	virtual void GenerateHeightMap3D() override;
	void CreateQuad(FMask Mask, FIntVector AxisMask, FIntVector V1, FIntVector V2, FIntVector V3, FIntVector V4);
	EBlock GetBlock(FIntVector Index) const;
	bool CompareMask(FMask M1, FMask M2) const;
	
};