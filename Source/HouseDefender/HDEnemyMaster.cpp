// Copyright 2021 DME Games


#include "HDEnemyMaster.h"
#include "TimerManager.h"

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

	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AHDEnemyMaster::OnBeginOverlap);
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &AHDEnemyMaster::OnEndOverlap);

	StartingLife = 100.f;
	MovementSpeed = 20.f;
}

// Called when the game starts or when spawned
void AHDEnemyMaster::BeginPlay()
{
	Super::BeginPlay();

	CurrentLife = StartingLife;

	GetPlayerCharacter();
	
}

// Called every frame
void AHDEnemyMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveTowardsPlayer(DeltaTime);
}

// Called to bind functionality to input
void AHDEnemyMaster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AHDEnemyMaster::OnBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap begun"));
}

void AHDEnemyMaster::OnEndOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap ended"));
}

void AHDEnemyMaster::SetHasBeenHit(const bool NewHitIn)
{
	bHasBeenHit = NewHitIn;

	if (GetHasBeenHit())
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HasBeenHit, this, &AHDEnemyMaster::ClearHasBeenHit, 0.67f, false, -1.f);
	}
}

bool AHDEnemyMaster::GetIsDead() const
{
	return CurrentLife <= 0.f;
}

void AHDEnemyMaster::SetCurrentLife(const float LifeTakenOff)
{
	CurrentLife -= LifeTakenOff;

	SetHasBeenHit(true);
	
	// Check if the enemy is dead and, after the delay for the death animation, destroy it
	if (GetIsDead())
	{
		GetWorldTimerManager().SetTimer(TimerHandle_EndOfLife, this, &AHDEnemyMaster::DestroyEnemy, 3.15f, false, -1.f);
	}
	// if enemy is not dead, play hit animation
//	else
//	{
//		SetHasBeenHit(true);
//	}
}

void AHDEnemyMaster::DestroyEnemy()
{
	// Ensure the timer is cleared before destroying the enemy
	GetWorldTimerManager().ClearTimer(TimerHandle_EndOfLife);
	this->Destroy();
}

void AHDEnemyMaster::ClearHasBeenHit()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_HasBeenHit);
	SetHasBeenHit(false);
}

void AHDEnemyMaster::MoveTowardsPlayer(float DeltaTime)
{
	if (PlayerCharacter && !GetIsDead() && !GetHasBeenHit())
	{
		const float NewActorX = GetActorLocation().X - (MovementSpeed * DeltaTime);
		const float CurrentActorY = GetActorLocation().Y;
		const float CurrentActorZ = GetActorLocation().Z;
		
		SetActorLocation(FVector(NewActorX, CurrentActorY, CurrentActorZ));
	}
}

void AHDEnemyMaster::GetPlayerCharacter()
{
	PlayerCharacter = Cast<AHDPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get player character"));
	}
}

