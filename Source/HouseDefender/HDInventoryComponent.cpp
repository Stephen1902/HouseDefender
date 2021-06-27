// Copyright 2021 DME Games


#include "HDInventoryComponent.h"
#include "HDItems.h"

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

bool UHDInventoryComponent::AddDroppedItem(const TSubclassOf<AHDItems> ItemClassIn, int32 QuantityIn)
{
	bool bItemAddedSuccessfully = false;
	
	if (ItemClassIn)
	{
		if (ItemMap.Find(ItemClassIn))
		{
			const int32 CurrentQty = ItemMap[ItemClassIn];
			ItemMap.Add(ItemClassIn, QuantityIn + CurrentQty);
			bItemAddedSuccessfully = true;
		}
		else
		{
			ItemMap.Add(ItemClassIn, QuantityIn);
			bItemAddedSuccessfully = true;
		}
	}

	return bItemAddedSuccessfully;
}

void UHDInventoryComponent::QueryInventory(const TSubclassOf<AHDItems> ItemClassIn, int32 QuantityIn, int32& QuantityOut, bool& SuccessOut)
{
	SuccessOut = false;
	QuantityOut = 0;

	if (ItemClassIn)
	{
		if (ItemMap.Find(ItemClassIn) && QuantityIn >= ItemMap[ItemClassIn])
		{
			SuccessOut = true;
			QuantityOut = *ItemMap.Find(ItemClassIn);
		}
	}
}

void UHDInventoryComponent::RemoveItemFromInventory(const TSubclassOf<AHDItems> ItemClassIn, int32 QuantityIn, int32& QuantityOut, bool& SuccessOut)
{
	SuccessOut = false;
	QuantityOut = 0;

	if (ItemClassIn)
	{
		int32 QuantityFromQuery = 0;
		bool SuccessFromQuery = false;

		QueryInventory(ItemClassIn, QuantityIn, QuantityFromQuery, SuccessFromQuery);

		if (SuccessFromQuery)
		{
			QuantityOut = QuantityIn - QuantityFromQuery;
			ItemMap.Add(ItemClassIn, QuantityOut);
			SuccessOut = true;
		}
	}
}


