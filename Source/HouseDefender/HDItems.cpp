// Copyright 2021 DME Games

#include "HDItems.h"
#include "HDGameStateBase.h"
#include "HDInventoryComponent.h"
#include "HDPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AHDItems::AHDItems()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Drop Mesh Component"));
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	SetRootComponent(ItemMesh);

	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle System Component"));
	ParticleSystemComponent->SetupAttachment(RootComponent);
	ParticleSystemComponent->SetRelativeLocation(FVector(0.f));

	MyTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));

	// Get the movement curve and populate the Blueprint info automatically if it is found with Constructor Helpers
	static ConstructorHelpers::FObjectFinder<UCurveFloat> CurveFloatObject(TEXT("/Game/Blueprints/WorldItems/Curves/FloatCurveNormal"));
	if (CurveFloatObject.Succeeded())
	{
		FCurve = CurveFloatObject.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get Curve Float in HDItems.cpp"))
	}

	// Get the item data table and populate the Blueprint info automatically if it is found with Constructor Helpers
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObject(TEXT("/Game/Blueprints/WorldItems/DT_ItemInfo.DT.DT_ItemInfo"));
	if (DataTableObject.Succeeded())
	{
		ItemInfoTable = DataTableObject.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get Data Table in HDItems.cpp"))
	}

	// Add functions to the timeline
	InterpFunction.BindUFunction(this, FName("TimelineFloatReturn"));
	TimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));

	// Set local ProductID to match Blueprint version
	CurrentProductID = ProductID;
}

void AHDItems::SetProductID(FName ProductIDIn)
{
	ProductID = ProductIDIn;
	const FString StringToDisplay = FName(ProductID).ToString();
	UE_LOG(LogTemp, Warning, TEXT("Set Product called in HD Items.  Product ID is %s"), *StringToDisplay);
	// Check for a valid data table
	if (ItemInfoTable)
	{
		// Find the row name to search for
		const FName RowName = ProductID;
		// Check that the row is valid based on the context string
		const FItemInformationTable* Row = ItemInfoTable->FindRow<FItemInformationTable>(RowName, "ItemId");

		if (Row)
		{
			ItemMesh->SetStaticMesh(Row->ItemDisplayMesh);
			if (Row->ItemType == EItemType::IT_Trap)
			{
				ItemMesh->SetCollisionObjectType(CHANNEL_Trap);
				ItemMesh->SetCollisionResponseToChannel(CHANNEL_Trap, ECR_Overlap);
				ItemMesh->SetCollisionResponseToChannel(CHANNEL_Enemy, ECR_Overlap);

				// Set the local slow rate variable
				EnemySlowRate =  Row->TrapInfo.EnemySlowRate;
				// Set the local Enemy Damage variable
				EnemyDamageRate = Row->TrapInfo.EnemyDamageRate;
				// Set the starting life of this item
				StartingLife = Row->TrapInfo.StartingLife;
				// Set how many hours this item needs to be repaired
				HoursNeededToRepair = Row->TrapInfo.HoursNeededToRepair;
			}

		}

	}
}

// Called when the game starts or when spawned
void AHDItems::BeginPlay()
{
	Super::BeginPlay();
	
	GetMovementCurve();
	GetReferences();	
	SetDropLocations();

	// TODO Remove as only for testing traps that are in the world
	const FString ProductIDAsString = FString::FromInt(0);
	SetProductID(FName(*ProductIDAsString));
}

void AHDItems::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsDestroyed && GetItemType() == EItemType::IT_Trap)
	{
		CheckForDamage(DeltaTime);
	}
}

void AHDItems::TimelineFloatReturn(float Value)
{
	SetActorLocation(FMath::Lerp(DropStartLocation, DropEndLocation, Value));
}

void AHDItems::OnTimelineFinished()
{
	Destroy();
}

// Function for changing information at design time only to auto populate information from the data table
#if WITH_EDITOR
void AHDItems::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Check the ProductID has been changed and that there is a valid ItemInfoTable
	if (ProductID != CurrentProductID && ItemInfoTable)
	{
		// Update so that the check above gets updated
		CurrentProductID = ProductID;

		// Find the row name to search for
		const FName RowName = ProductID;
		// Check that the row is valid based on the context string
		FItemInformationTable* Row = ItemInfoTable->FindRow<FItemInformationTable>(RowName, "ItemId");

		if (Row)
		{
			ItemType = Row->ItemType;
		}
	}
}
#endif

void AHDItems::GameStateChanged()
{
	if (GameStateBaseRef && FCurve && ItemType != EItemType::IT_Trap)
	{
		// At the end of the day, move the drop to the designated area	
		MyTimeline->Play();
		
		if (PlayerCharacterRef)
		{
			// A check, returned by the inventory component that the item added successfully
			bool SuccessReturned = false;

			if (UHDInventoryComponent* PInventory = PlayerCharacterRef->PlayerInventory)
			{
				PInventory->AddDroppedItem(ProductID, NumberOfItemsToAdd, SuccessReturned);
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
}

void AHDItems::GetReferences()
{
	GameStateBaseRef = Cast<AHDGameStateBase>(GetWorld()->GetGameState());

	if (GameStateBaseRef)
	{
		GameStateBaseRef->OnStatusChanged.AddDynamic(this, &AHDItems::GameStateChanged);
		GameStateBaseRef->SetEndOfDayDelayTime(MaxCurveTimeRange);
	}
	
	PlayerCharacterRef = Cast<AHDPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	
} 

void AHDItems::SetDropLocations()
{
	if (PlayerCharacterRef && ItemType != EItemType::IT_Trap)
	{
		DropStartLocation = GetActorLocation();
		DropEndLocation = PlayerCharacterRef->GetDropLocation();
	}
}

void AHDItems::CheckForDamage(float DeltaTime)
{
	if (!FMath::IsNearlyZero(CurrentDamageBeingDealt))
	{
		GEngine->AddOnScreenDebugMessage(0, 0.f, FColor::Green, FString::FromInt(CurrentLife));
		CurrentLife -= (CurrentDamageBeingDealt * DeltaTime);
		
		if (CurrentLife <= 0.f && !bIsDestroyed)
		{
			OnTrapDestroyed();
		}
/** Keep only in case of having a visible health widget 
		else if (WidgetComp)
		{
			if (!WidgetComp->IsVisible())
			{
				WidgetComp->SetVisibility(true);
			}
			const float CurrentLifeAsPercent = 1 - ((StartingLife - CurrentLife) / StartingLife);
			OnTrapHit.Broadcast(UserWidget, CurrentLifeAsPercent);
		}
*/
	}
}

void AHDItems::OnTrapDestroyed()
{
	bIsDestroyed = true;

	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ItemMesh->SetGenerateOverlapEvents(false);
	CurrentDamageBeingDealt = 0.f;

/** Keep only in case of having a visible health widget 	
	WidgetComp->SetVisibility(false);
*/
	this->Destroy();
}

void AHDItems::GetTrapInfo(float& SlowsEnemyRateOut, float& DamagesEnemyRateOut, float& HasLifeAmountOut, float& RepairTimeOut) const
{
	SlowsEnemyRateOut = EnemySlowRate;
	DamagesEnemyRateOut = EnemyDamageRate;
	HasLifeAmountOut = StartingLife;
	RepairTimeOut = HoursNeededToRepair;
}

void AHDItems::AddDamageToTrap(float DamageToAddIn)
{
	CurrentDamageBeingDealt += DamageToAddIn;
}
