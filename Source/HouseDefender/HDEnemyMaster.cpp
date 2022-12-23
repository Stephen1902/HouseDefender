// Copyright 2021 DME Games

#include "HDEnemyMaster.h"
#include "TimerManager.h"
#include "HDAIController.h"
#include "HDItems.h"
#include "HDGameStateBase.h"
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
	CapsuleComponent->SetCollisionObjectType(ECollisionChannel::CHANNEL_Enemy);
	CapsuleComponent->SetCollisionResponseToChannel(CHANNEL_Enemy, ECR_Ignore);

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	MeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	
	MovementComponent = CreateDefaultSubobject<UPawnMovementComponent>(TEXT("Movement Component"));

	AIControllerClass = AHDAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	// Set the default values for the on screen health bar widget
	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	WidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComp->SetDrawSize(FVector2D(100.f, 20.f));
	WidgetComp->SetupAttachment(MeshComponent);
	WidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	ColourOfHealthBar = FLinearColor(1.f, 0.125f, 0.125f, 3.0f);

	StartingLife = 100.f;
	DefaultMovementSpeed = 20.f;
	SpeedReductionFromTraps = 1.0f;

	// Get the item data table and populate the Blueprint info automatically if it is found with Constructor Helpers
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObject(TEXT("/Game/Blueprints/WorldItems/DT_ItemInfo.DT.DT_ItemInfo"));
	if (DataTableObject.Succeeded())
	{
		ItemInfoTable = DataTableObject.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get Data Table in AHDEnemyMaster.cpp"))
	}

	// Get the game state so the dropped item array is ready within the editor
	// No longer needed as using data table not array in game state but keeping the code for future reference on how to get the game state in a constructor
	// GetGameState();

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

	// Add functions to be called when the capsule is overlapped by a trap
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AHDEnemyMaster::OnCapsuleBeginOverlap);
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &AHDEnemyMaster::OnCapsuleEndOverlap);

	CurrentMovementSpeed = DefaultMovementSpeed;
}
#if WITH_EDITOR
void AHDEnemyMaster::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	bool bChangeMade = false;
	// Check that the local array has at least one entry and that there is a valid Item Info Data Table
	if (DroppableItemsList.Num() > 0  && ItemInfoTable)
	{
		// Loop through the Dropped Item Array to check against the data table for this item
		for (int32 i = 0; i < DroppableItemsList.Num(); ++i)
		{
			// Only check if a change hasn't already been found since only one array element can be changed at a time
			if (!bChangeMade)
			{
				FItemInformationTable* Row = ItemInfoTable->FindRow<FItemInformationTable>(DroppableItemsList[i].ItemIdToDrop, "Item Details");
				// Check that the row is valid and the name in the array doesn't match the name in the data table
				if (Row && Row->ItemName.ToString() != DroppableItemsList[i].ItemNameToDrop.ToString())
				{
					bChangeMade = true;
					DroppableItemsList[i].ItemNameToDrop = Row->ItemName;
					DroppableItemsList[i].bIsCrafted = Row->bIsCrafted;
				}

			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Droppable Item List is still zero or there is no Item Data Table in AHDEnemyMaster.cpp"));
	}
}
#endif

// Called every frame
void AHDEnemyMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveTowardsPlayer(DeltaTime);
	UpdateWidgetLocation();
	CheckForDamage(DeltaTime);
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
		// Clear the blocked variable to show the hit animation
		SetIsBlocked(false);
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

void AHDEnemyMaster::SetSpeedReductionFromTraps(const float SpeedIn)
{
	SpeedReductionFromTraps = SpeedIn;
}

void AHDEnemyMaster::SetDamageTaken(const float DamageIn)
{
	DamageTaken = DamageIn;
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

				// Set the spawn location for the dropped actor
				FVector SpawnLocation = GetActorLocation();
				// Set the Z location so the actor spawns on the ground
				SpawnLocation.Z = GetActorLocation().Z - CapsuleComponent->GetScaledCapsuleHalfHeight();
				// Set the spawn rotation or the dropped actor
				FRotator RandomSpawnRotation = GetActorRotation();
				// Set the yaw so dropped items don't always appear side on to the player
				RandomSpawnRotation.Yaw += FMath::FRandRange(10.0f, 80.f);
				
				AHDItems* ItemToAdd = GetWorld()->SpawnActor<AHDItems>(AHDItems::StaticClass(), SpawnLocation, RandomSpawnRotation, SpawnParameters);
				ItemToAdd->SetProductID(It.ItemIdToDrop);

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
	// Clear the timer for the hit animation
	GetWorldTimerManager().ClearTimer(TimerHandle_HasBeenHit);
	SetHasBeenHit(false);	
}

void AHDEnemyMaster::MoveTowardsPlayer(float DeltaTime)
{
	if (PlayerCharacter && !GetIsDead() && !GetHasBeenHit() && !GetIsBlocked())
	{
		const float NewActorX = GetActorLocation().X - (CurrentMovementSpeed * (DeltaTime * SpeedReductionFromTraps));
		const float CurrentActorY = GetActorLocation().Y;
		const float CurrentActorZ = GetActorLocation().Z;
		
		SetActorLocation(FVector(NewActorX, CurrentActorY, CurrentActorZ));
	}
}

void AHDEnemyMaster::GetPlayerCharacter()
{
	// Try to get the player character, return an error if not
	PlayerCharacter = Cast<AHDPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get player character in HDEnemyMaster.cpp"));
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

void AHDEnemyMaster::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCapsuleBeginOverlap Called"))
	// Check if what has been overlapped is another item ie not another enemy
	if (AHDItems* ItemOverlapped = Cast<AHDItems>(OtherActor))
	{
	// Check if what has been overlapped is a trap
		if (ItemOverlapped->GetItemType() == EItemType::IT_Trap)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has overlapped %s"), *GetName(), *ItemOverlapped->GetName());
			ItemOverlapped->AddDamageToTrap(DamageDealtToTraps);
			// Set the reference to the trap that enemy has "hit"
			OverlappedTrapRef = ItemOverlapped;
			// Tell the enemy it has hit something, playing the hit animation
			SetHasBeenHit(true);
			// Tell the enemy it is blocked until the trap has been destroyed
			SetIsBlocked(true);

			// Get the required information from the trap, setting it to this instance of the enemy
			float SpeedReductionIn, DamageRateIn, HasLifeAmountIn, RepairTimeIn;
			ItemOverlapped->GetTrapInfo(SpeedReductionIn, DamageRateIn, HasLifeAmountIn, RepairTimeIn);
			SetSpeedReductionFromTraps(SpeedReductionIn);
			SetDamageTaken(DamageRateIn);

			UE_LOG(LogTemp, Warning, TEXT("SpeedReductionIn: %s, DamageRateIn: %s"), *FString::FromInt(SpeedReductionIn), *FString::FromInt(DamageRateIn));
		}
	}
}

void AHDEnemyMaster::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCapsuleEndOverlap Called"))
	// Check what the end overlap is and if it is of the type HDItems
	if (const AHDItems* ItemOverlapped = Cast<AHDItems>(OtherActor))
	{
	// Check if what has been overlapped is a of the type, trap
		if (ItemOverlapped->GetItemType() == EItemType::IT_Trap)
		{
			// Set the reference to a nullptr
			OverlappedTrapRef = nullptr;

			// Reset the GetIsBlocked variable so the enemy can move forward again
			SetIsBlocked(false);

			// Reset the variables that can be affected by a trap
			SetSpeedReductionFromTraps(1.f);
			SetDamageTaken(0.f);
		}
	}
}

void AHDEnemyMaster::CheckForDamage(float DeltaTime)
{
	if (!FMath::IsNearlyZero(DamageTaken))
	{
		SetCurrentLife(DamageTaken * DeltaTime);
	}
}

/* No longer used but kept here to show how to get a reference in the constructor
void AHDEnemyMaster::GetGameState()
{
	// Get the Game State if it is found with Constructor Helpers and set it inside the 
	static ConstructorHelpers::FClassFinder<AHDGameStateBase> GameStateBase(TEXT("/Script/HouseDefender.HDGameStateBase"));
	if (GameStateBase.Succeeded())
	{
		const TSubclassOf<AHDGameStateBase> LocalGameStateRef = GameStateBase.Class;
		GameStateRef = Cast<AHDGameStateBase>(LocalGameStateRef.GetDefaultObject());
	}

	if (!GameStateRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to set GameStateRef in HDEnemyMaster.cpp"))
	}
}
*/