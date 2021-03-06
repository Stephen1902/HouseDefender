// Copyright 2021 DME Games


#include "HDEnemyMaster.h"
#include "TimerManager.h"
#include "HDAIController.h"
#include "HDItems.h"
#include "HDInteractableMaster.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"

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

// Kept here in case of later use
//	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AHDEnemyMaster::OnBeginOverlap);
//	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &AHDEnemyMaster::OnEndOverlap);

	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	WidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComp->SetDrawSize(FVector2D(100.f, 20.f));
	WidgetComp->SetupAttachment(MeshComponent);
	WidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	ColourOfHealthBar = FLinearColor(1.f, 0.125f, 0.125f, 3.0f);
	
	StartingLife = 100.f;
	MovementSpeed = 20.f;
	SpeedReductionFromTraps = 1.0f;
}

// Called when the game starts or when spawned
void AHDEnemyMaster::BeginPlay()
{
	Super::BeginPlay();

	CurrentLife = StartingLife;

	
	GetPlayerCharacter();
	SetWidgetInfo();

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
	CheckForDamage(DeltaTime);
}

// Called to bind functionality to input
void AHDEnemyMaster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

/** Kept here in case of later use
void AHDEnemyMaster::OnBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Pears Overlap begun %s"), *OtherActor->GetName());
}

void AHDEnemyMaster::OnEndOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Pears Overlap ended"));
}
*/

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
	if (!FMath::IsNearlyZero(LifeTakenOff) && !GetIsDead())
	{
		CurrentLife -= LifeTakenOff;

		// Update the enemy widget
		UpdateWidgetInformation();

		if (!GetIsBlocked())
		{
			SetHasBeenHit(true);
		}
		else
		{
			SetHasBeenHit(false);
		}
		
		// Check if the enemy is dead  and destroy them, using IsTimerActive to only call this once
		if (GetIsDead() && !GetWorldTimerManager().IsTimerActive(TimerHandle_EndOfLife)) 
		{
			SetHasBeenHit(false);
			SetIsBlocked(false);
			CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			WidgetComp->SetVisibility(false);
			GetWorldTimerManager().SetTimer(TimerHandle_EndOfLife, this, &AHDEnemyMaster::DestroyEnemy, 3.15f, false, -1.f);
		}
	}
}

void AHDEnemyMaster::DestroyEnemy()
{
	// Check the enemy has items in its inventory and check if we are to spawn them
	if (DroppableItemsList.Num() > 0)
	{
		//const TArray<FInventoryItems> LocalItemArray = InventoryComponent->GetInventoryItems(); 
		for (auto& It : DroppableItemsList)
		{
			// Get a random number which, if it is less than the probability of spawning, then spawn them
			const float SpawnChance = FMath::RandRange(0.f, 1.f);
			if (SpawnChance < It.DropProbability)
				
			{
				// Spawn a visible actor into the world					
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				ensure(It.ItemToDrop);

				FVector SpawnLocation = GetActorLocation();
				SpawnLocation.Z = GetActorLocation().Z;
				AHDItems* ItemToAdd = GetWorld()->SpawnActor<AHDItems>(It.ItemToDrop, SpawnLocation, GetActorRotation(), SpawnParameters);

				// Check maximum amount that can be spawned
				const float SpawnAmount = FMath::RandRange(1, It.MaxDropAmount);				
				ItemToAdd->SetNumberOfItemsToAdd(SpawnAmount);
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
		const float NewActorX = GetActorLocation().X - (MovementSpeed * (DeltaTime * SpeedReductionFromTraps));
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
	if (!WidgetComp->IsVisible())
	{
		WidgetComp->SetVisibility(true);
	}

	const float CurrentLifeAsPercent = 1 - ((StartingLife - CurrentLife) / StartingLife);
	OnEnemyHit.Broadcast(UserWidget, CurrentLifeAsPercent);
}

void AHDEnemyMaster::SetWidgetInfo()
{
	if (WidgetComp)
	{
		WidgetLocationAtStart = WidgetComp->GetRelativeLocation();
		UserWidget = WidgetComp->GetUserWidgetObject();
		WidgetComp->SetVisibility(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WidgetInfo has not been set for %s"), *GetName());
	}
}

void AHDEnemyMaster::CheckForDamage(float DeltaTime)
{
	if (!FMath::IsNearlyZero(DamageTaken))
	{
		SetCurrentLife(DamageTaken * DeltaTime);
	}
}

