// Copyright 2021 DME Games


#include "HDInventoryComponent.h"

// Sets default values for this component's properties
UHDInventoryComponent::UHDInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	MaxCoins = -1.f;
}

void UHDInventoryComponent::AddDroppedItem(const FName ProductIdIn, int32 QuantityIn, bool& SuccessOut)
{
	// Check if the ProductID already exists in the map
	if (ItemMap.Find(ProductIdIn))
	{
		// Overwrite the existing product ID with the current quantity plus quantity to be added
		const int32 CurrentQty = ItemMap[ProductIdIn];		
		ItemMap.Add(ProductIdIn, QuantityIn + CurrentQty);
	}
	else
	{
		// Create the key with the quantity added
		ItemMap.Add(ProductIdIn, QuantityIn);
	}

	SuccessOut = true;
}

void UHDInventoryComponent::QueryInventory(const FName ProductIdIn, int32 QuantityIn, int32& QuantityOut, bool& SuccessOut)
{
	if (ItemMap.Find(ProductIdIn))// && QuantityIn >= ItemMap[ProductIdIn])
	{
		SuccessOut = true;
		QuantityOut = *ItemMap.Find(ProductIdIn);
	}
	else
	{
		SuccessOut = false;
		QuantityOut = 0;
	}
}

void UHDInventoryComponent::RemoveItemFromInventory(const FName ProductIdIn, int32 QuantityIn, int32& QuantityOut, bool& SuccessOut)
{
	int32 QuantityFromQuery = 0;
	bool SuccessFromQuery = false;

	// Get the quantity the player has of the item needed to craft the item from ProductIdIn. Stored in QuantityFromQuery.
	QueryInventory(ProductIdIn, QuantityIn, QuantityFromQuery, SuccessFromQuery);

	if (SuccessFromQuery)
	{
		// Remove the amount needed from the player inventory
		QuantityOut = QuantityFromQuery - QuantityIn;
		// Update the Map with the new quantity
		ItemMap.Add(ProductIdIn, QuantityOut);
		SuccessOut = true;
	}
	else
	{
		SuccessOut = false;
		QuantityOut = 0;
	}
}


