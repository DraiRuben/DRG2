#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Chunks/Data/Enums.h"
#include "ItemData.generated.h"

UENUM()
enum class EItemRarity:uint8
{
	Bad UMETA(DisplayName = "Bad"),
	Common UMETA(DisplayName = "Common"),
	Rare UMETA(DisplayName = "Rare"),
	Epic UMETA(DisplayName = "Epic"),
	Legendary UMETA(DisplayName = "Legendary"),
};

UENUM()
enum class EItemType : uint8
{
	Armor UMETA(DisplayName = "Armor"),
	Weapon UMETA(DisplayName = "Weapon"),
	Consumable UMETA(DisplayName = "Consumable"),
	Block UMETA(DisplayName = "Block"),
	Misc UMETA(DisplayName = "Miscellaneous")
};

USTRUCT()
struct FItemStatistics
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere)
	float ArmorRating;
	UPROPERTY(EditAnywhere)
	float DamageValue;
	UPROPERTY(EditAnywhere)
	float HealValue;
	UPROPERTY(EditAnywhere)
	EBlock BlockType;
};

USTRUCT()
struct FItemDescriptionData
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere)
	FText Name;
	UPROPERTY(EditAnywhere)
	FText Description;
	UPROPERTY(EditAnywhere)
	FText InteractionText;
	
};

USTRUCT()
struct FItemNumericData
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere)
	bool IsStackable;
	UPROPERTY(EditAnywhere)
	int32 StackLimit = 64;
};

USTRUCT()
struct FItemAssetData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;
	UPROPERTY(EditAnywhere)
	UTexture2D* Icon;
	UPROPERTY(EditAnywhere)
	UStaticMesh* Mesh;
};
USTRUCT()
struct FItemData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FName ID;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	EItemRarity ItemRarity;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	EItemType ItemType;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemStatistics Statistics;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemAssetData AssetData;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemDescriptionData DescriptionData;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemNumericData NumericData;
};