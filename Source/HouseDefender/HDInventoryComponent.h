// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HDInventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HOUSEDEFENDER_API UHDInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHDInventoryComponent();

	float GetMaxCoins() const { return MaxCoins; }

	TMap<FName, int32> GetItemMap() { return ItemMap; }

	// Check the existing array and add quantity or new item, depending on requirement
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddDroppedItem(const FName ProductIdIn, int32 QuantityIn, bool& SuccessOut);

	// Get the current items in the inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void QueryInventory(const FName ProductIdIn, int32 QuantityIn, int32& QuantityOut, bool& SuccessOut);

	// Remove items from inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemFromInventory(const FName ProductIdIn, int32 QuantityIn, int32& QuantityOut, bool& SuccessOut);
	
protected:
	// Map for storing items in the player inventory and their quantity
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	TMap<FName, int32> ItemMap;

	/* Maximum amount of coins that can be carried, -1 is infinite */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
	float MaxCoins;
};
