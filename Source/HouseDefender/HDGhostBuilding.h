// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HD_BlueprintFunctionLibrary.h"
#include "HDGhostBuilding.generated.h"

#define CHANNEL_TrapTrace ECC_GameTraceChannel1

UCLASS()
class HOUSEDEFENDER_API AHDGhostBuilding : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHDGhostBuilding();

	UPROPERTY(EditDefaultsOnly, Category = "Ghost Building")
	UStaticMeshComponent* StaticMeshComp;

	void SetReferences(class AHDPlayerController* PlayerControllerIn);

	UFUNCTION(BlueprintCallable, Category = "Ghost Building")
	void OnSpawn(FName ProductIDToSpawnIn, class AHDConstructionProxy* ConstructionProxyIn);

	UFUNCTION(BlueprintCallable, Category = "Ghost Building")
	void KillGhost();

	UFUNCTION(BlueprintCallable, Category = "Ghost Building")
	void SetMousePositionInWorld(const FVector PositionInFromController) { MousePositionInWorld = PositionInFromController; }

	FConstructionStruct GetConstructionStruct() const { return ConstructionStruct; }

	// Checks if this item is overlapping anything
	bool CheckForOverlap() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// A reference to the player controller
	UPROPERTY(VisibleAnywhere, Category = "Ghost Building")
	class AHDPlayerController* PlayerControllerRef;

	// A reference to the fake construction item, used so the item being placed can be easily destroyed if cancelled
	UPROPERTY(VisibleAnywhere, Category = "Ghost Building")
	class AHDConstructionProxy* ConstructionProxyRef;

	// Information storage for all game items
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	UDataTable* ItemInfoTable;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	FName ProductIDToSpawn;

	UPROPERTY()
	UStaticMesh* DisplayMesh;

	// Names of the rows found in the item data table
	UPROPERTY()
	TArray<FName> RowNames;

	bool bIsActive;

	FVector MousePositionInWorld;

	float GetGridAxisSnap(float AxisIn, int32 GridIn) const;
	
	// Used to snap the crafted items to a virtual grid to avoid being able to place things too close together
	FVector SetGridSnap() const;

	// Struct of current info to be passed to other classes
	UPROPERTY()
	FConstructionStruct ConstructionStruct;

	// Material Instance to be applied to each mesh for the faded effect when placing
	UPROPERTY()
	class UMaterialInstanceDynamic* MaterialInstanceRef;

	UPROPERTY()
	UMaterial* MaterialObjectRef;

};
