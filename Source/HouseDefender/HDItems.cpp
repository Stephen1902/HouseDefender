// Copyright 2021 DME Games

#include "HDItems.h"
#include "HDGameStateBase.h"
#include "HDInventoryComponent.h"
#include "HDPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TimelineComponent.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AHDItems::AHDItems()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Drop Mesh Component"));
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	SetRootComponent(ItemMesh);

	MyTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));

	static ConstructorHelpers::FObjectFinder<UCurveFloat> CurveFloatObject(TEXT("/Game/Blueprints/WorldItems/Curves/FloatCurveNormal"));
	FCurve = CurveFloatObject.Object;

	InterpFunction.BindUFunction(this, FName("TimelineFloatReturn"));
	TimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));
}

// Called when the game starts or when spawned
void AHDItems::BeginPlay()
{
	Super::BeginPlay();

	GetMovementCurve();
	GetReferences();	
	SetDropLocations();
	
}

void AHDItems::TimelineFloatReturn(float Value)
{
	SetActorLocation(FMath::Lerp(DropStartLocation, DropEndLocation, Value));
}

void AHDItems::OnTimelineFinished()
{
	Destroy();
}

void AHDItems::GameStateChanged()
{
	if (GameStateBaseRef && FCurve && ItemType != EItemType::IT_Trap)
	{
		// At the end of the day, move the drop to the designated area	
		MyTimeline->Play();
		
		if (PlayerCharacter)
		{
			if (UHDInventoryComponent* PInventory = PlayerCharacter->PlayerInventory)
			{
				PInventory->AddDroppedItem(this->GetClass(), NumberOfItemsToAdd);
			}
		}
	}	
}

void AHDItems::GetMovementCurve()
{
	if (FCurve)
	{
		// Update function
		MyTimeline->AddInterpFloat(FCurve, InterpFunction, FName("Alpha"));
		// OnFinished function
		MyTimeline->SetTimelineFinishedFunc(TimelineFinished);

		// Set the timeline settings
		MyTimeline->SetLooping(false);
		MyTimeline->SetIgnoreTimeDilation(true);

		float MinCurveTimeRange;
		FCurve->GetTimeRange(MinCurveTimeRange, MaxCurveTimeRange);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Float Curve was not found.  Check it's location in the code"));
	}

}

void AHDItems::GetReferences()
{
	GameStateBaseRef = Cast<AHDGameStateBase>(GetWorld()->GetGameState());

	if (GameStateBaseRef)
	{
		GameStateBaseRef->OnStatusChanged.AddDynamic(this, &AHDItems::GameStateChanged);
		GameStateBaseRef->SetEndOfDayDelayTime(MaxCurveTimeRange);
	}
	
	PlayerCharacter = Cast<AHDPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	
}

void AHDItems::SetDropLocations()
{
	if (PlayerCharacter && ItemType != EItemType::IT_Trap)
	{
		DropStartLocation = GetActorLocation();
		DropEndLocation = PlayerCharacter->GetDropLocation();
	}

	
}
