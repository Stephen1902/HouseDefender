// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HDDrops.generated.h"

UCLASS()
class HOUSEDEFENDER_API AHDDrops : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHDDrops();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	class UStaticMeshComponent* ItemMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	class UTexture2D* ItemThumbnail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	FText ItemDescription;

	// Dictates whether or not the item can be traded
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	bool bCanBeTraded;
	
	// Value of the item when selling or buying
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items", meta = (EditCondition = "bCanBeTraded"))
	float ItemValue;
	
protected:
	// World item to be added to the inventory when this pick up is collected
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "World Item")
	AHDDrops* DroppedItem;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
