#pragma once

UENUM(BlueprintType)
enum class EDirection : uint8
{
	Forward =0, Right=1, Back=2, Left=3, Up=4, Down=5
};

UENUM(BlueprintType)
enum class EBlock : uint8
{
	Null, Air, Stone, Dirt, Grass
};
