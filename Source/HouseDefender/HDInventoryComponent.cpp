// Copyright 2021 DME Games


#include "HDInventoryComponent.h"
#include "HDDrops.h"

// Sets default values for this component's properties
UHDInventoryComponent::UHDInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	MaxCoins = -1.f;
}

// Called when the game starts
void UHDInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UHDInventoryComponent::AddDroppedItem(const TSubclassOf<AHDDrops> ItemClassIn)
{
	if (ItemClassIn)
	{
		bool bItemNeedsAdding = true;

		if (InventoryItems.Num() > 0)
		{
			for (auto& It : InventoryItems)
			{
				if (ItemClassIn == It.ItemClass)
				{
					It.MaxCanSpawn += 1;
					bItemNeedsAdding = false;
				}
			}
		}
		
		if (bItemNeedsAdding)
		{
			FInventoryItems NewItem;
			NewItem.ItemClass = ItemClassIn;
			NewItem.ItemProbability = 1.f;
			NewItem.MaxCanSpawn = 1;
			InventoryItems.Add(NewItem);			
		}
	}
}