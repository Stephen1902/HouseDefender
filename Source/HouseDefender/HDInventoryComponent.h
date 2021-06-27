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

	/* Maximum amount of coins that can be carried, -1 is infinite */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Inventory")
	float MaxCoins;

	float GetMaxCoins() const { return MaxCoins; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Inventory")
	TMap<TSubclassOf<class AHDItems>, int32> ItemMap;

	// Check the existing array and add quantity or new item, depending on requirement
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddDroppedItem(const TSubclassOf<class AHDItems> ItemClassIn, int32 QuantityIn);

	// Get the current items in the inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void QueryInventory(const TSubclassOf<class AHDItems> ItemClassIn, int32 QuantityIn, int32& QuantityOut, bool& SuccessOut);

	// Remove items from inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemFromInventory(const TSubclassOf<class AHDItems> ItemClassIn, int32 QuantityIn, int32& QuantityOut, bool& SuccessOut);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

};
