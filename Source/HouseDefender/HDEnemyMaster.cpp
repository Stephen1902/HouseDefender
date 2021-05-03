// Copyright 2021 DME Games


#include "HDEnemyMaster.h"
#include "TimerManager.h"
#include "HDAIController.h"
#include "HDDrops.h"
#include "HDInteractableMaster.h"
#include "HDInventoryComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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

	InventoryComponent = CreateDefaultSubobject<UHDInventoryComponent>(TEXT("Inventory Component"));

	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	WidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComp->SetDrawSize(FVector2D(100.f, 20.f));
	WidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	
	StartingLife = 100.f;
	MovementSpeed = 20.f;
}

// Called when the game starts or when spawned
void AHDEnemyMaster::BeginPlay()
{
	Super::BeginPlay();

	CurrentLife = StartingLife;
	WidgetLocationAtStart = WidgetComp->GetRelativeLocation();
	WidgetComp->SetVisibility(false);
	
	GetPlayerCharacter();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHDInteractableMaster::StaticClass(), FoundActors);
	// TODO Set up stairs and code what happens when enemy collides with them
}

// Called every frame
void AHDEnemyMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveTowardsPlayer(DeltaTime);
	UpdateWidgetLocation();
}

// Called to bind functionality to input
void AHDEnemyMaster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AHDEnemyMaster::OnBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap begun %s"), *OtherActor->GetName());
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

	// Update the enemy widget
	UpdateWidgetInformation();

	// Play the hit animation
	SetHasBeenHit(true);
	
	// Check if the enemy is dead and, after the delay for the death animation, destroy it
	if (GetIsDead())
	{
		WidgetComp->SetVisibility(false);
		GetWorldTimerManager().SetTimer(TimerHandle_EndOfLife, this, &AHDEnemyMaster::DestroyEnemy, 3.15f, false, -1.f);
	}
}

void AHDEnemyMaster::DestroyEnemy()
{
	// Check the enemy has items in its inventory and check if we are to spawn them
	if (InventoryComponent->InventoryItems.Num() > 0)
	{
		const TArray<FInventoryItems> LocalItemArray = InventoryComponent->GetInventoryItems(); 
		for (int32 i = 0; i < LocalItemArray.Num(); ++i)
		{
			// Get a random number which, if it is less than the probability of spawning, then spawn them
			const float SpawnChance = FMath::RandRange(0.f, 1.f);
			if (SpawnChance < LocalItemArray[i].ItemProbability)
			{
				// Check maximum amount that can be spawned
				const float SpawnAmount = FMath::RandRange(1, LocalItemArray[i].MaxCanSpawn);
				
				for (int j = 1; j <= SpawnAmount; ++j)
				{
					
					FActorSpawnParameters SpawnParameters;
					SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

					ensure(LocalItemArray[i].ItemClass);

					FVector SpawnLocation = GetActorLocation();
					SpawnLocation.Z = GetActorLocation().Z;
					GetWorld()->SpawnActor<AHDDrops>(LocalItemArray[i].ItemClass, SpawnLocation, GetActorRotation(), SpawnParameters);					
				}
			}
		}
	}
	
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

void AHDEnemyMaster::UpdateWidgetLocation() const
{
	const FVector CurrentEnemyLocation = MeshComponent->GetRelativeLocation();
	const float NewZHeight = (CapsuleComponent->GetUnscaledCapsuleHalfHeight() * 2) + 40.f; 
	WidgetComp->SetRelativeLocation(FVector(CurrentEnemyLocation.X, CurrentEnemyLocation.Y, CurrentEnemyLocation.Z + NewZHeight));
}

void AHDEnemyMaster::UpdateWidgetInformation() const
{
	if (!WidgetComp->IsWidgetVisible())
	{
		WidgetComp->SetVisibility(true);
	}

	const float CurrentLifeAsPercent = 1 - ((StartingLife - CurrentLife) / StartingLife);
	UUserWidget* UserWidget = WidgetComp->GetUserWidgetObject();
	OnEnemyHit.Broadcast(UserWidget, CurrentLifeAsPercent);
}

