#include "OptimisedChunk.h"

#include "Enums.h"

AOptimisedChunk::AOptimisedChunk()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOptimisedChunk::BeginPlay()
{
	Super::BeginPlay();
}

void AOptimisedChunk::GenerateMesh()
{
	for (int Axis = 0; Axis < 3; ++Axis)
	{
		const int Axis1 = (Axis+1)%3;
		const int Axis2 = (Axis+2)%3;

		const int MainAxisLimit = Size[Axis];
		int Axis1Limit = Size[Axis1];
		int Axis2Limit = Size[Axis2];

		auto DeltaAxis1 = FIntVector::ZeroValue;
		auto DeltaAxis2 = FIntVector::ZeroValue;

		auto ChunkItr = FIntVector::ZeroValue;
		auto AxisMask = FIntVector::ZeroValue;

		AxisMask[Axis] = 1;
		TArray<FMask> Mask;
		Mask.SetNum(Axis1Limit * Axis2Limit);

		for(ChunkItr[Axis] = -1; ChunkItr[Axis] <MainAxisLimit; )
		{
			int N = 0;
			for(ChunkItr[Axis2] = 0 ; ChunkItr[Axis2] < Axis2Limit;++ChunkItr[Axis2])
			{
				for(ChunkItr[Axis1] = 0 ; ChunkItr[Axis1] < Axis1Limit;++ChunkItr[Axis1])
				{
					const auto CurrentBlock = GetBlock(ChunkItr);
					const auto CompareBlock = GetBlock(ChunkItr + AxisMask);
					
					const bool CurrentBlockOpaque = CurrentBlock != EBlock::Air;
					const bool CompareBlockOpaque = CompareBlock != EBlock::Air;
					if(CurrentBlockOpaque == CompareBlockOpaque)
					{
						Mask[N++] = FMask{EBlock::Null, 0};
					}else if(CurrentBlockOpaque)
					{
						Mask[N++] = FMask{CurrentBlock, 1};
					}else
					{
						Mask[N++] = FMask{CompareBlock, -1};
					}
				}
			}
			++ChunkItr[Axis];
			N=0;

			for(int j = 0;j<Axis2Limit;++j)
			{
				for(int i =0;i<Axis1Limit;)
				{
					if(Mask[N].Normal !=0)
					{
						const auto CurrentMask = Mask[N];
						ChunkItr[Axis1] = i;
						ChunkItr[Axis2] = j;
						int width;
						for(width = 1;i+width <Axis1Limit && CompareMask(Mask[N+width],CurrentMask);++width)
						{
							
						}
						int height;
						bool done = false;
						for(height = 1; j + height< Axis2Limit;++height)
						{
							for(int k = 0; k<width;++k)
							{
								if(CompareMask(Mask[N+k + height* Axis1Limit],CurrentMask)) continue;
								done = true;
								break;
							}
							if(done) break;
						}

						DeltaAxis1[Axis1] = width;
						DeltaAxis2[Axis2] = height;
						CreateQuad(CurrentMask, AxisMask,
							ChunkItr,
							ChunkItr + DeltaAxis1,
							ChunkItr + DeltaAxis2,
							ChunkItr + DeltaAxis1+ DeltaAxis2);

						DeltaAxis1 = FIntVector::ZeroValue;
						DeltaAxis2 = FIntVector::ZeroValue;

						for(int l = 0;l<height;++l)
						{
							for(int k = 0;k<width;++k)
							{
								Mask[N+k+l*Axis1Limit]= FMask{EBlock::Null,0};
							}
						}
						i+= width;
						N+= width;
					}else
					{
						i++;
						N++;
					}
				}
			}
		}
		
	}
}

void AOptimisedChunk::CreateQuad(FMask Mask,
	FIntVector AxisMask,
	FIntVector V1,
	FIntVector V2,
	FIntVector V3,
	FIntVector V4)
{
	const auto Normal = FVector(AxisMask * Mask.Normal);
	ChunkData.VertexData.Append({
		FVector(V1) * 100,
		FVector(V2) * 100,
		FVector(V3) * 100,
		FVector(V4) * 100
	});

	ChunkData.TriangleData.Append({
		VertexCount,
		VertexCount + 2 + Mask.Normal,
		VertexCount + 2 - Mask.Normal,
		VertexCount + 3,
		VertexCount + 1 - Mask.Normal,
		VertexCount + 1 + Mask.Normal
	});

	ChunkData.UVData.Append({
		FVector2D(0,0),
		FVector2D(0,1),
		FVector2D(1,0),
		FVector2D(1,1)
	});

	ChunkData.Normals.Append({
		Normal,
		Normal,
		Normal,
		Normal
	});
	const auto Color = FColor::MakeRandomColor();
	ChunkData.Colors.Append({ Color, Color, Color, Color });

	VertexCount+=4;
}

EBlock AOptimisedChunk::GetBlock(FIntVector Index) const
{
	if (Index.X >= Size.X || Index.Y >= Size.Y || Index.Z >= Size.Z ||Index.X < 0 || Index.Y < 0 || Index.Z < 0)
	{
		return EBlock::Air;
	}
	return Blocks[GetBlockIndex(Index.X,Index.Y,Index.Z)];
}

bool AOptimisedChunk::CompareMask(FMask M1, FMask M2) const
{
	return M1.Block == M2.Block && M1.Normal == M2.Normal;
}

