// Copyright 2021 DME Games

#include "HDDrops.h"
#include "HDGameStateBase.h"
#include "HDInventoryComponent.h"
#include "HDPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TimelineComponent.h"

// Sets default values
AHDDrops::AHDDrops()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Drop Mesh Component"));
	ItemMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SetRootComponent(ItemMesh);

	MyTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));

	InterpFunction.BindUFunction(this, FName("TimelineFloatReturn"));
	TimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));

}

void AHDDrops::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MyTimeline->IsPlaying())
	{
		GEngine->AddOnScreenDebugMessage(0, 0.f, FColor::Green, TEXT("Timer Is Running"));
	}
}

// Called when the game starts or when spawned
void AHDDrops::BeginPlay()
{
	Super::BeginPlay();

	GameStateBaseRef = Cast<AHDGameStateBase>(GetWorld()->GetGameState());

	if (GameStateBaseRef)
	{
		GameStateBaseRef->OnStatusChanged.AddDynamic(this, &AHDDrops::GameStateChanged);
	}
	
	PlayerCharacter = Cast<AHDPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	
	if (PlayerCharacter)
	{
		DropEndLocation = PlayerCharacter->GetDropLocation()->GetActorLocation();
	}

	if (FCurve)
	{
		// Update function
		MyTimeline->AddInterpFloat(FCurve, InterpFunction, FName("Alpha"));
		// OnFinished function
		MyTimeline->SetTimelineFinishedFunc(TimelineFinished);

		// Set the timeline settings
		MyTimeline->SetLooping(false);
		MyTimeline->SetIgnoreTimeDilation(true);
	}

	
	DropStartLocation = GetActorLocation();
}

void AHDDrops::TimelineFloatReturn(float Value)
{
	SetActorLocation(FMath::Lerp(DropStartLocation, DropEndLocation, Value));
}

void AHDDrops::OnTimelineFinished()
{
	Destroy();
}

void AHDDrops::GameStateChanged()
{
	if (GameStateBaseRef && FCurve)
	{
		// At the end of the day, move the drop to the designated area	
		MyTimeline->Play();
		
		if (PlayerCharacter)
		{
			if (UHDInventoryComponent* PInventory = PlayerCharacter->PlayerInventory)
			{
				PInventory->AddDroppedItem(this->GetClass());
			}
		}
	}	
}
