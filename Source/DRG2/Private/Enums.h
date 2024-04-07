#pragma once

UENUM(BlueprintType)
enum class EDirection : uint8
{
	Forward =0, Right=1, Back=2, Left=3, Up=4, Down=5
};
static int GetInverseDirection(int Direction)
{
	return Direction>=4 ? Direction%2 == 0 ? Direction+1:Direction-1: Direction>=2 ?Direction-2:Direction+2;
}
UENUM(BlueprintType)
enum class EBlock : uint8
{
	Null, Air, Stone, Dirt, Grass, Trunk, Leaves, Water
};

UENUM(BlueprintType)
enum class EGenerationType:uint8
{
	Gen2D, Gen3D, GenComplete
};