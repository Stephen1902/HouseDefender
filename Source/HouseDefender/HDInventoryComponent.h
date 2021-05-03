// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HDInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItems
{
	GENERATED_BODY()

	/* World items carried by this inventory component */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="World Item")
	TSubclassOf<class AHDDrops> ItemClass;
	
	/* The probability between 0 and 1 of this item being dropped or available from the trader */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="World Item")
	float ItemProbability;

	/* The maximum number that can be spawned by the owner of this inventory */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="World Item", meta = (ClampMin = 1))
	int32 MaxCanSpawn;

	FInventoryItems()
	{
		ItemProbability = 0.5f;
		MaxCanSpawn = 1;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOUSEDEFENDER_API UHDInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHDInventoryComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Inventory")
	TArray<FInventoryItems> InventoryItems;

	/* Maximum amount of coins that can be carried, -1 is infinite */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Inventory")
	float MaxCoins;

	float GetMaxCoins() const { return MaxCoins; }

	TArray<FInventoryItems> GetInventoryItems() const { return InventoryItems; }

	// Check the existing array and add quantity or new item, depending on requirement
	void AddDroppedItem(const TSubclassOf<class AHDDrops> ItemClassIn);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

};
