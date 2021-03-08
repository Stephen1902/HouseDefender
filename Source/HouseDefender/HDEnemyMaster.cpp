// Copyright 2021 DME Games


#include "HDEnemyMaster.h"


#include "HDAIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
AHDEnemyMaster::AHDEnemyMaster()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
	SetRootComponent(CapsuleComponent);
	CapsuleComponent->SetCapsuleHalfHeight(88.f);
	CapsuleComponent->SetCapsuleRadius(34.f);
	CapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	MeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	
	MovementComponent = CreateDefaultSubobject<UPawnMovementComponent>(TEXT("Movement Component"));

	AIControllerClass = AHDAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AHDEnemyMaster::BeginPlay()
{
	Super::BeginPlay();

	
}

// Called every frame
void AHDEnemyMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AHDEnemyMaster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

