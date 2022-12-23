// Copyright 2021 DME Games

#include "HD_WidgetDayEnd.h"
#include "HDGameStateBase.h"
#include "HDPlayerCharacter.h"
#include "HDPlayerController.h"
#include "Kismet/GameplayStatics.h"

UHD_WidgetDayEnd::UHD_WidgetDayEnd(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	// Get the item data table and populate the Blueprint info automatically if it is found with Constructor Helpers
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObject(TEXT("/Game/Blueprints/WorldItems/DT_ItemInfo.DT.DT_ItemInfo"));
	if (DataTableObject.Succeeded())
	{
		ItemInfoTable = DataTableObject.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get Data Table in HDWidgetDayEnd.cpp"))
	}

}

bool UHD_WidgetDayEnd::Initialize()
{
	Super::Initialize();

	GameStateRef = Cast<AHDGameStateBase>(GetWorld()->GetGameState());
	
	PlayerCharacterRef = Cast<AHDPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	PlayerControllerRef = Cast<AHDPlayerController>(UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0));
	
	// Check that both the GameStateRef and PlayerCharacterRef were set correctly
	if (!GameStateRef || !PlayerCharacterRef || !PlayerControllerRef)
	{
		if (!GameStateRef)
		{
			UE_LOG(LogTemp, Warning, TEXT("HD_WidgetDayEnd failed to get the GameState"));
		}

		if (!PlayerCharacterRef)
		{
			UE_LOG(LogTemp, Warning, TEXT("HD_WidgetDayEnd failed to get the Player Character"));
		}

		if (!PlayerControllerRef)
		{
			UE_LOG(LogTemp, Warning, TEXT("HD_WidgetDayEnd failed to get the PlayerControllerRef"));
		}

		return false;
	}
	else
	{
		// Get the game items which are marked as bIsCrafted
		SetCraftedItemArray();

		return true;
	}


}

FItemInformationTable UHD_WidgetDayEnd::GetCurrentProduct(FName ProductIdIn)
{
	FItemInformationTable* CurrentProduct = nullptr;

	FItemInformationTable* Row = ItemInfoTable->FindRow<FItemInformationTable>(ProductIdIn, "Item Details");
	if (Row)
	{
		CurrentProduct = Row;
	}

	return *CurrentProduct;
}

void UHD_WidgetDayEnd::SpawnNewItem(FName ProductIDToSpawn)
{
	//FunctionLibraryRef->TestFunction();
	PlayerControllerRef->SpawnNewItem(ProductIDToSpawn);
}

void UHD_WidgetDayEnd::SetCraftedItemArray()
{
	if (GameStateRef)
	{
		CraftedItemArray = GameStateRef->GetCraftedGameItems();
	}
}
