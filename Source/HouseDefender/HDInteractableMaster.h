// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HDInteractableMaster.generated.h"

UCLASS()
class HOUSEDEFENDER_API AHDInteractableMaster : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHDInteractableMaster();

	UPROPERTY(EditDefaultsOnly, Category = "SetUp")
	class UStaticMeshComponent* MeshComponent;

	// Name to be used when detecting what item has been hit
	UPROPERTY(EditDefaultsOnly, Category = "SetUp")
	FText ItemName;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	FText GetItemName() const { return ItemName; }
};
