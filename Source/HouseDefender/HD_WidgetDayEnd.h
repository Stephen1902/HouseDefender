// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HD_BlueprintFunctionLibrary.h"
#include "HD_WidgetDayEnd.generated.h"

/**
 * 
 */
UCLASS()
class HOUSEDEFENDER_API UHD_WidgetDayEnd : public UUserWidget
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHD_WidgetDayEnd(const FObjectInitializer& ObjectInitializer);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category="References")
	class AHDGameStateBase* GameStateRef;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	class AHDPlayerCharacter* PlayerCharacterRef;

	UPROPERTY(BlueprintReadOnly, Category = "References")
	class AHDPlayerController* PlayerControllerRef;


	UPROPERTY()
	UHD_BlueprintFunctionLibrary* FunctionLibraryRef;

	// Called when the game starts
	virtual bool Initialize() override;

	// Information storage for all game items
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Items")
	UDataTable* ItemInfoTable;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Items")
	AActor* DayViewCameraLocation;

	UFUNCTION(BlueprintCallable, Category = "Items")
	const TArray<FItemInformationTable> GetCraftedItemArray() { return CraftedItemArray; }

	UFUNCTION(BlueprintCallable, Category = "Items")
	FItemInformationTable GetCurrentProduct(FName ProductIdIn);

	UFUNCTION(BlueprintCallable, Category = "Items")
	void SpawnNewItem(FName ProductIDToSpawn);

private:
	void SetCraftedItemArray();

	UPROPERTY()
	TArray<FItemInformationTable> CraftedItemArray;


};
