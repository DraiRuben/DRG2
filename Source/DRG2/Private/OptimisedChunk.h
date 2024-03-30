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
	void CreateQuad(const int VoxelType, FMask Mask, FIntVector AxisMask,const int Width, const int Height, FIntVector V1, FIntVector V2, FIntVector V3, FIntVector V4);
	EBlock GetBlock(FIntVector Index) const;
	bool CompareMask(FMask M1, FMask M2) const;
	
};