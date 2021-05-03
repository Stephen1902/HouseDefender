// Copyright 2021 DME Games

#include "HDDrops.h"
#include "HDInventoryComponent.h"
#include "HDPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AHDDrops::AHDDrops()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Drop Mesh Component"));
	ItemMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SetRootComponent(ItemMesh);

}

// Called when the game starts or when spawned
void AHDDrops::BeginPlay()
{
	Super::BeginPlay();

	if (AHDPlayerCharacter* PlayerCharacter = Cast<AHDPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn()))
	{
		if (UHDInventoryComponent* PInventory = PlayerCharacter->PlayerInventory)
		{
			PInventory->AddDroppedItem(this->GetClass());
			// TODO add visual effect to move item an on screen bag
		}
	}
}
