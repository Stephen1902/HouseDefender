// Copyright 2021 DME Games


#include "HDGameStateBase.h"
#include "HDItems.h"

AHDGameStateBase::AHDGameStateBase()
{
	// Get the item data table and populate the Blueprint info automatically if it is found with Constructor Helpers
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObject(TEXT("/Game/Blueprints/WorldItems/DT_ItemInfo.DT.DT_ItemInfo"));
	if (DataTableObject.Succeeded())
	{
		ItemInfoTable = DataTableObject.Object;

		AddArrayItems();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get Data Table in AHDGameStateBase.cpp"))
	}
}

void AHDGameStateBase::SetGameStatus(const EGameStatus GameStatusIn)
{
	GameStatus = GameStatusIn;

	OnStatusChanged.Broadcast();
}

float AHDGameStateBase::GetEndOfDayDelayTime() const
{
	return EndOfDayDelayTime;
}
/*
TArray<AHDItems*> AHDGameStateBase::GetItemSubClasses()
{
	TArray<AHDItems*> Results;
	
	
	UClass* HDItems = AHDItems::StaticClass();

	if (HDItems)
	{
		// get our parent blueprint class
		const FString ParentClassName= HDItems->GetName();
		UObject* ClassPackage = ANY_PACKAGE;
		AHDItems* ParentCPPClass = FindObject<AHDItems>(ClassPackage, *ParentClassName);

		// iterate over UClass, this might be heavy on performance
		for (TObjectIterator<AHDItems> It; It; ++It)
		{
//			if (It->class(ParentCPPClass))
			{
				// It is a child of the Parent Class
				// make sure we don't include our parent class in the array (weak name check, suggestions welcome)
				if (It->GetName() != ParentClassName)
				{
					Results.AddUnique(*It);
					

					//  TRY TSUBCLASS OF!!!!!!!!!!!!!!!!

				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Parent class is not valid"));
	}
	
	if (Results.Num() > 0)
	{
		for (int32 i = 0; i < Results.Num(); ++i)
		{
			UE_LOG(LogTemp, Warning, TEXT("Class found: %s"), *Results[i]->GetFullName())

			
		}
	}
	return Results;
}

TArray<UClass*> AHDGameStateBase::GetItemClasses()
{
	TArray<UClass*> Results;
	/*
	UClass* HDItems = AHDItems::StaticClass();

	if (HDItems)
	{
		// get our parent blueprint class
		const FString ParentClassName= HDItems->GetName();
		UObject* ClassPackage = ANY_PACKAGE;
		UClass* ParentCPPClass = FindObject<UClass>(ClassPackage, *ParentClassName);

		// iterate over UClass, this might be heavy on performance, so keep in mind..
		// better suggestions for a check are welcome
		for (TObjectIterator<UClass> It; It; ++It)
		{
			//  Original code before trying something new
			//if (It->IsChildOf(ParentCPPClass))
			if (It->IsChildOf(AHDItems::StaticClass()))
			{
				AHDItems* CurrentItem = Cast<AHDItems>(It->GetDefaultObject());

				if (CurrentItem->bIsCrafted)
				{
					CraftedItemArray.Add(*It);
				}
				else
				{
					//NonCraftedItemArray.Add(*It);
				}

			
				// It is a child of the Parent Class
				// make sure we don't include our parent class in the array (weak name check, suggestions welcome)
			//	if (It->GetName() != ParentClassName)
				{
					//Results.AddUnique(*It);
					Results.Add(*It);
				}
				
			}
			
		}
	}


// Get all objects in the world
	for (TObjectIterator<UClass> It; It; ++It)
	{
		//  Check if the object found is a class taken from HDItems
		if (It->IsChildOf(AHDItems::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract))
		{
		//	UE_LOG(LogTemp, Warning, TEXT("Class found: %s"), *It->GetName())
			Results.Add(*It);
		}
	}

	if (Results.Num() > 0)
	{
		for (int32 It = 0; It < Results.Num(); ++It)
		
		{
			
		}
	}
	
	return Results;
	
}
*/
void AHDGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	
/*
	GetItemClasses();
	GetItemSubClasses();
*/



//	int32 HDItemsNum = GetItemClasses().Num();
//	int32 HDSubItemsAsNum = GetItemSubClasses().Num();
//	UE_LOG(LogTemp, Warning, TEXT("Number of HDItems %i"), GetItemClasses().Num());
//	UE_LOG(LogTemp, Warning, TEXT("Number of SubItems %i"), GetItemSubClasses().Num());
}

void AHDGameStateBase::AddArrayItems()
{
	const FString ContextString;
	// Get all Row Names in the data table and add them to an array
	TArray<FName> RowNames = ItemInfoTable->GetRowNames();

	for (const auto& Name : RowNames)
	{
		// Iterate through the array and find a matching row in the table
		const FItemInformationTable* Row = ItemInfoTable->FindRow<FItemInformationTable>(Name, ContextString);
		// Check for a valid row
		if (Row)
		{
			// Check whether the item is crafted or not, adding to the relevant array
			if (Row->bIsCrafted)
			{
				CraftedGameItems.Add(*Row);
			}
			else
			{
				DroppedGameItems.Add(*Row);
			}
		}
	}
}
