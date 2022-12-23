// Copyright 2021 DME Games

#include "HDGhostBuilding.h"
#include "HDConstructionProxy.h"
#include "HDItems.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "HDPlayerController.h"

// Sets default values
AHDGhostBuilding::AHDGhostBuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Comp"));
	SetRootComponent(StaticMeshComp);
	StaticMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaticMeshComp->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);

	// Get the item data table and populate the Blueprint info automatically if it is found with Constructor Helpers
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObject(TEXT("/Game/Blueprints/WorldItems/DT_ItemInfo.DT.DT_ItemInfo"));
	if (DataTableObject.Succeeded())
	{
		ItemInfoTable = DataTableObject.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get Data Table in AHDGhostBuilding.cpp"))
	}

	// Get a reference to the material needed
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialObject(TEXT("Material'/Game/Materials/M_GhostBuilding.M_GhostBuilding'"));
	if (MaterialObject.Succeeded())
	{
		MaterialObjectRef = MaterialObject.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find the material needed for HDGhostBuilding"));
	}
}

void AHDGhostBuilding::SetReferences(AHDPlayerController* PlayerControllerIn)
{
	PlayerControllerRef = PlayerControllerIn;
}

void AHDGhostBuilding::OnSpawn(FName ProductIDToSpawnIn, AHDConstructionProxy* ConstructionProxyIn)
{
	if (!bIsActive)
	{
		ProductIDToSpawn = ProductIDToSpawnIn;
		ConstructionProxyRef = ConstructionProxyIn;
		bIsActive = true;

		if (ItemInfoTable)
		{
			// Find the row name to search for
			const FName RowName = ProductIDToSpawn;
			// Check that the row is valid based on the context string
			FItemInformationTable* Row = ItemInfoTable->FindRow<FItemInformationTable>(RowName, "ItemId");

			if (Row)
			{
				StaticMeshComp->SetStaticMesh(Row->ItemDisplayMesh);

				// Check if the item being spawned is shown in the Data Table as a trap
				if (Row->ItemType == EItemType::IT_Trap)
				{
					//  Set it as a trap for collision purposes
					StaticMeshComp->SetCollisionObjectType(CHANNEL_TrapTrace);
					// Apply the semi-opaque material
					if (MaterialObjectRef)
					{
						MaterialInstanceRef = UMaterialInstanceDynamic::Create(MaterialObjectRef, this);
						StaticMeshComp->SetMaterial(0, MaterialInstanceRef);
					}
					
				}

				// Set Parameters that don't change when the items is spawned into the struct for use later
				ConstructionStruct.BuildingID = ProductIDToSpawn;
				ConstructionStruct.ProxyClassRef = ConstructionProxyRef;
				ConstructionStruct.DisplayMesh = Row->ItemDisplayMesh;
			}

		}


/*		// Iterate through the array of row names and find a match for the Product ID that came in
		for (const auto& Name : RowNames)
		{
			UE_LOG(LogTemp, Warning, TEXT("Looping"))
			const FString ContextString;
			const FItemInformationTable* Row = ItemInfoTable->FindRow<FItemInformationTable>(Name, "ItemId");
			// Check for a valid row
			if (Row)
			{
				// Set the display mesh if one is found
				StaticMeshComp->SetStaticMesh(Row->ItemDisplayMesh);

				UE_LOG(LogTemp, Warning, TEXT("Found a valid row "));
			}
		}*/
	}
}

void AHDGhostBuilding::KillGhost()
{
	const FString ResetIDString = TEXT("-1");
	ProductIDToSpawn = *ResetIDString;
	ConstructionProxyRef = nullptr;
	bIsActive = false;
	StaticMeshComp->SetStaticMesh(nullptr);
}

// Called when the game starts or when spawned
void AHDGhostBuilding::BeginPlay()
{
	Super::BeginPlay();

	// Get all Row Names in the data table
	RowNames = ItemInfoTable->GetRowNames();

}

// Called every frame
void AHDGhostBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive)
	{
		SetActorLocation(SetGridSnap());
		//SetActorLocation(MousePositionInWorld);

		// Update ConstructionStruct items that change when moved
		ConstructionStruct.GhostBuildingLocation = GetActorLocation();
		ConstructionStruct.GhostBuildingRotation = GetActorRotation();

		if (CheckForOverlap())
		{
			MaterialInstanceRef->SetVectorParameterValue(TEXT("Colour"), FVector(1.0f, 0.f, 0.f));
		}
		else
		{
			MaterialInstanceRef->SetVectorParameterValue(TEXT("Colour"), FVector(1.0f, 1.f, 1.f));
		}
	}

}

float AHDGhostBuilding::GetGridAxisSnap(float AxisIn, int32 GridIn) const
{
	// The mouse cursor will be in the middle of the mesh so the grid size needs to reflect this
	const float AxisWithAddition = AxisIn + (GridIn / 2);
	const float AxisToFloor = AxisWithAddition / GridIn;
	const int32 FlooredAxis = FMath::Floor(AxisToFloor);
	const float AxisToReturn = FlooredAxis * GridIn;

	return AxisToReturn;
}

FVector AHDGhostBuilding::SetGridSnap() const
{
	constexpr int32 GridSnapSize = 100.f;

	// Clamp the X Value so that it cannot go too far to the left or right of screen
	const float SnapPosX = FMath::Clamp(GetGridAxisSnap(MousePositionInWorld.X, GridSnapSize), 1000.f, 2800.f);
	const float SnapPosZ = MousePositionInWorld.Z;

	return FVector(SnapPosX, 0.f, SnapPosZ);
}

bool AHDGhostBuilding::CheckForOverlap() const
{
	TArray<AActor*> OverlappingItems;
	StaticMeshComp->GetOverlappingActors(OverlappingItems, AHDItems::StaticClass());

	const FString TextToDisplay = "Overlapping: " + FString::FromInt(OverlappingItems.Num());
	GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Red, *TextToDisplay, false);

	return OverlappingItems.Num() > 0;
}
