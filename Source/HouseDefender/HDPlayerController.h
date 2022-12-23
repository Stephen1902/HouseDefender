// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "HD_BlueprintFunctionLibrary.h"
#include "GameFramework/PlayerController.h"
#include "HDPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrapBeingPlaced, const bool, IsBuilding);

#define Collision_Landscape	ECC_GameTraceChannel2

/**
 * 
 */
UCLASS()
class HOUSEDEFENDER_API AHDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SpawnNewItem(FName ProductIDToSpawnIn);

	void SetBuildModeActive(const bool BuildModeIn) { bBuildModeActive = BuildModeIn;}

	// Used with Multicast delegate above for when the player is placing a crafted item
	UPROPERTY(BlueprintAssignable, Category="Delegates")
	FOnTrapBeingPlaced OnTrapBeingPlaced;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupInputComponent() override;

	UPROPERTY()
	class AHDGameStateBase* GameStateRef;

	UPROPERTY()
	class AHDPlayerCharacter* PlayerCharacterRef;
private:
	UPROPERTY()
	class AHDConstructionProxy* ConstructionProxyRef;

	UPROPERTY()
	class AHDGhostBuilding* GhostBuildingRef;

	bool bBuildModeActive = false;

	// Gets the world position of the mouse to use with placing crafted items into the world
	FVector SetCursorWorldPosition() const;

	// Distance the player can move away from the camera before the build function stops working
	float SightDistance = 10000.f;

	// Places the current item in the world
	void LMBClicked();

	// Cancels the current item being spawned without placing it in the world
	void CancelSpawn();

	// ConstructionStruct, info supplied by the GhostBuilding class
	UPROPERTY()
	FConstructionStruct ConstructionStruct;

	void GetReferences();

	// Take the GhostBuilding and try to place an actual object in the world
	void TryToPlaceItem();

	// Tell the player to fire if they can
	void TryToFire() const;
};