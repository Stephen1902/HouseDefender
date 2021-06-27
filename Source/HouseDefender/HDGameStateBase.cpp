// Copyright 2021 DME Games


#include "HDGameStateBase.h"

#include "HDItems.h"

void AHDGameStateBase::SetGameStatus(const EGameStatus GameStatusIn)
{
	GameStatus = GameStatusIn;

	OnStatusChanged.Broadcast();
}

float AHDGameStateBase::GetEndOfDayDelayTime() const
{
	return  EndOfDayDelayTime;
}

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

		// iterate over UClass, this might be heavy on performance, so keep in mind..
		// better suggestions for a check are welcome
		for (TObjectIterator<AHDItems> It; It; ++It)
		{
//			if (It->class(ParentCPPClass))
			{
				// It is a child of the Parent Class
				// make sure we don't include our parent class in the array (weak name check, suggestions welcome)
	//			if (It->GetName() != ParentClassName)
				{
					Results.Add(*It);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Parent class is not valid"));
	}
	
	return Results;
}

TArray<UClass*> AHDGameStateBase::GetItemClasses()
{
	TArray<UClass*> Results;
	
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
			if (It->IsChildOf(ParentCPPClass))
			{
				// It is a child of the Parent Class
				// make sure we don't include our parent class in the array (weak name check, suggestions welcome)
				if (It->GetName() != ParentClassName)
				{
					Results.Add(*It);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Parent class is not valid"));
	}
	
	return Results;

}
