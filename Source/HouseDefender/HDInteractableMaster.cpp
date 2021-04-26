// Copyright 2021 DME Games


#include "HDInteractableMaster.h"

// Sets default values
AHDInteractableMaster::AHDInteractableMaster()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Comp"));

	ItemName = FText::FromString("Default Name");
}

// Called when the game starts or when spawned
void AHDInteractableMaster::BeginPlay()
{
	Super::BeginPlay();
	
}

