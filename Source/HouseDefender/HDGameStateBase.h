// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
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

	UFUNCTION(BlueprintCallable, Category = "Crafting Subclass")
	static TArray<class AHDItems*> GetItemSubClasses();

	UFUNCTION(BlueprintCallable, Category = "Crafting Subclass")
	static TArray<UClass*> GetItemClasses();
	
	void SetEndOfDayDelayTime(const float DelayTimeIn) { EndOfDayDelayTime = DelayTimeIn; }
private:
	float EndOfDayDelayTime;
};
