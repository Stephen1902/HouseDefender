// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HD_BlueprintFunctionLibrary.h"
#include "HDGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatusChanged);

/**
 * 
 */
UENUM(BlueprintType)
enum class EGameStatus : uint8
{
	GS_Idle			UMETA(DisplayName = "Idle"),
	GS_DayStarting	UMETA(DisplayName = "Day is starting"),
	GS_DayStarted	UMETA(DisplayName = "Day has started"),
	GS_DayEnding	UMETA(DisplayName = "Day is ending")
};

UCLASS()
class HOUSEDEFENDER_API AHDGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	// Sets the defaults for the Game State
	AHDGameStateBase();

	UPROPERTY(EditAnywhere, Category = "Game State")
	EGameStatus GameStatus;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	EGameStatus GetGameStatus() { return GameStatus; }

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetGameStatus(EGameStatus GameStatusIn);

	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnStatusChanged OnStatusChanged;

	UFUNCTION(BlueprintCallable, Category = "Game State Tester")
	float GetEndOfDayDelayTime() const;


	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Game State")
		TArray<FItemInformationTable> DroppedGameItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
		TArray<FItemInformationTable> CraftedGameItems;
/*
	UFUNCTION(BlueprintCallable, Category = "Crafting Subclass")
	static TArray<class AHDItems*> GetItemSubClasses();

	UFUNCTION(BlueprintCallable, Category = "Crafting Subclass")
	static TArray<UClass*> GetItemClasses();
*/	
	void SetEndOfDayDelayTime(const float DelayTimeIn) { EndOfDayDelayTime = DelayTimeIn; }

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	TArray<FItemInformationTable> GetDroppedGameItems() const { return DroppedGameItems;}

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	TArray<FItemInformationTable> GetCraftedGameItems() const { return CraftedGameItems; }


	/*
	UFUNCTION(BlueprintCallable, Category = "World Items")
	const TArray<class AHDItems*> GetCraftedItems();

	UFUNCTION(BlueprintCallable, Category = "World Items")
	const TArray<class AHDItems*> GetNonCraftedItems();
	*/
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category="Arrays")
	TArray<AHDItems*> CraftedItemArray;

	// Information storage for all game items
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	UDataTable* ItemInfoTable;

private:
	float EndOfDayDelayTime;

	void AddArrayItems();
/*
	UPROPERTY()
	TArray<FItemInformationTable> DroppedGameItems;

	UPROPERTY()
	TArray<FItemInformationTable> CraftedGameItems;
	*/
	
};
