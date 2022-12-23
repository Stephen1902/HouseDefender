// Copyright 2021 DME Games

#include "HDPlayerController.h"
#include "HDPlayerCharacter.h"
#include "HDConstructionProxy.h"
#include "HDGameStateBase.h"
#include "HDGhostBuilding.h"
#include "HDItemsTraps.h"
#include "Kismet/GameplayStatics.h"


void AHDPlayerController::SpawnNewItem(FName ProductIDToSpawnIn)
{
	if (GhostBuildingRef)
	{
		// Remove all previous information in case building was cancelled before placing
		GhostBuildingRef->KillGhost();

		// Update the Ghost with new information
		GhostBuildingRef->OnSpawn(ProductIDToSpawnIn, ConstructionProxyRef);
	}
	/*
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnLoc = FVector(0.f);
	const FRotator SpawnRot = FRotator(0.f);

	AHDItems* BuildingToSpawn = GetWorld()->SpawnActor<AHDItems>(AHDItems::StaticClass(), SpawnLoc, SpawnRot, SpawnInfo);
	BuildingToSpawn->SetProductID(ProductIDToSpawnIn);
	*/

	bBuildModeActive = true;

	OnTrapBeingPlaced.Broadcast(true);
}

void AHDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	GetReferences();
}

void AHDPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bBuildModeActive)
	{
		const FVector MouseCursorPosition = SetCursorWorldPosition();
		if (MouseCursorPosition != FVector(0.f) && GhostBuildingRef)
		{
			GhostBuildingRef->SetMousePositionInWorld(MouseCursorPosition);
		}

	}
}

void AHDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("CancelCraftedItem", IE_Pressed, this, &AHDPlayerController::CancelSpawn);
	InputComponent->BindAction("LMBClicked", IE_Pressed, this, &AHDPlayerController::LMBClicked);
}

FVector AHDPlayerController::SetCursorWorldPosition() const
{
	FVector MousePos;
	FVector MouseDir;
	DeprojectMousePositionToWorld(MousePos, MouseDir);

	const FVector StartLoc = MousePos;
	const FVector EndLoc = ((MouseDir * SightDistance) + MousePos);

	FHitResult Hit;
	FCollisionQueryParams CollParams;

	GetWorld()->LineTraceSingleByChannel(Hit, StartLoc, EndLoc, ECC_GameTraceChannel2, CollParams);

	if (Hit.GetActor())
	{
		return Hit.Location;
	}

	return FVector(0.f);
}

void AHDPlayerController::LMBClicked()
{
	// Check for a valid Game State
	if (GameStateRef)
	{
		// Check the ENUM and see if the player should fire or place an item in the world
		switch (GameStateRef->GetGameStatus())
		{
		case EGameStatus::GS_Idle:
			TryToPlaceItem();
			break;
		case EGameStatus::GS_DayEnding:
			break;
		case EGameStatus::GS_DayStarted:
			TryToFire();
			break;
		case EGameStatus::GS_DayStarting:
			break;
		}
	}

}

void AHDPlayerController::CancelSpawn()
{
	GhostBuildingRef->KillGhost();
	bBuildModeActive = false;
	SetShowMouseCursor(true);
	OnTrapBeingPlaced.Broadcast(false);
}

void AHDPlayerController::GetReferences()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnLoc = FVector(0.f);
	const FRotator SpawnRot = FRotator(0.f);

	ConstructionProxyRef = GetWorld()->SpawnActor<AHDConstructionProxy>(AHDConstructionProxy::StaticClass(), SpawnLoc, SpawnRot, SpawnInfo);
	GhostBuildingRef = GetWorld()->SpawnActor<AHDGhostBuilding>(AHDGhostBuilding::StaticClass(), SpawnLoc, SpawnRot, SpawnInfo);

	if (GhostBuildingRef)
	{
		// Set this controller as a reference inside HDGhostBuilding
		GhostBuildingRef->SetReferences(this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to set GhostBuildingRef in HDPlayerController"));
	}

	GameStateRef = Cast<AHDGameStateBase>(GetWorld()->GetGameState());
	PlayerCharacterRef = Cast<AHDPlayerCharacter>(GetPawn());

	if (!GameStateRef || !PlayerCharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get either the GameStateRef or PlayerCharacterRef in HDPlayerController"))
	}
}

void AHDPlayerController::TryToPlaceItem()
{
	// Check we're in build mode and have a valid reference to the Ghost Building class
	if (bBuildModeActive && GhostBuildingRef)
	{
		// Check that the item we are trying to place isn't colliding with anything else
		if (!GhostBuildingRef->CheckForOverlap())
		{
			// Get the information from the Ghost Building class
			ConstructionStruct = GhostBuildingRef->GetConstructionStruct();

			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			if (AHDItemsTraps* NewlyPlacedItem = GetWorld()->SpawnActor<AHDItemsTraps>(AHDItemsTraps::StaticClass(), ConstructionStruct.GhostBuildingLocation, ConstructionStruct.GhostBuildingRotation, SpawnParameters))
			{
				NewlyPlacedItem->SetProductID(ConstructionStruct.BuildingID);
				NewlyPlacedItem->SetControllerRef(this);
				// If the item has been successfully spawned, cancel
				CancelSpawn();

			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to spawn HDItem in HDPlayerController"));
			}
		}
	}
}

void AHDPlayerController::TryToFire() const
{
	// Check for a valid Player Character Reference
	if (PlayerCharacterRef)
	{
		// The day is being run, tell the player to fire
		PlayerCharacterRef->TryToFire();
	}
}


