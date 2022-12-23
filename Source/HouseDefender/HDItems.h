 // Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "HD_BlueprintFunctionLibrary.h"
#include "HDItems.generated.h"

#define CHANNEL_Trap	ECC_GameTraceChannel1
#define CHANNEL_Enemy	ECC_GameTraceChannel3

/*
UENUM(BlueprintType)
enum class EItemType : uint8
{
	IT_Plants		UMETA(DisplayName = "Medicinal Part"),
	IT_TrapPart		UMETA(DisplayName = "Trap Part"),
	IT_AmmoPart		UMETA(DisplayName = "Ammo Part"),
	IT_Medicinal	UMETA(DisplayName = "Medicinal"),
	IT_Trap			UMETA(DisplayName = "Trap"),
	IT_Ammo			UMETA(DisplayName = "Ammo")
};
*/


UCLASS()
class HOUSEDEFENDER_API AHDItems : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHDItems();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	UParticleSystemComponent* ParticleSystemComponent;
	
	// Information storage for all game items
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items")
	UDataTable* ItemInfoTable;

	// ID as taken from the data table as to what this item actually is
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items")
	FName ProductID;

	// The type of item this is (Trap part, Ammo Part, etc...) as taken from the data table based on ProductID
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	EItemType ItemType;
	
	EItemType GetItemType() const { return ItemType; }

	// Called by the class that spawns the item
	void SetProductID(FName ProductIDIn);

/*
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



	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	bool bIsCrafted;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items", meta = (EditCondition = "bIsCrafted"))
	TMap<TSubclassOf<AHDItems>, int32> NeededForCrafting; 
*/
	UPROPERTY()
	UTimelineComponent* MyTimeline;

	void SetNumberOfItemsToAdd(int32 NumberToAddIn) { NumberOfItemsToAdd = NumberToAddIn; }
	void SetControllerRef(class AHDPlayerController* PlayerControllerIn) { PlayerControllerRef = PlayerControllerIn; }
	// Information to be given to the EnemyMasterHD for any enemy that interacts with this item if it is a trap
	void GetTrapInfo(float &SlowsEnemyRateOut, float &DamagesEnemyRateOut, float &HasLifeAmountOut, float &RepairTimeOut) const;

	void AddDamageToTrap(float DamageToAddIn);
protected:
	// World item to be added to the inventory when this pick up is collected
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "World Item")
	AHDItems* DroppedItem;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void TimelineFloatReturn(float Value);

	UFUNCTION()
	void OnTimelineFinished();

	/** Declare a delegate to call with TimeLineFloatReturn */
	FOnTimelineFloat InterpFunction{};

	/** Declare a delegate to call with OnTimelineFinished */
	FOnTimelineEvent TimelineFinished{};

	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY()
	class AHDGameStateBase* GameStateBaseRef;

	UPROPERTY()
	class AHDPlayerCharacter* PlayerCharacterRef;

	UPROPERTY()
	class AHDPlayerController* PlayerControllerRef;

	UFUNCTION()
	void GameStateChanged();

	FVector DropStartLocation;
	FVector DropEndLocation;

	// Curve used when moving object dropped by an enemy into the edge of the screen and the player's inventory	
	UPROPERTY()
	class UCurveFloat* FCurve;

	// Time taken for curve to complete in seconds
	float MaxCurveTimeRange;

	// Functions for game start
	void GetMovementCurve();
	void GetReferences();
	void SetDropLocations();

	int32 NumberOfItemsToAdd;

	// Current ItemID so info does not get changed if the value of ProductID has not been changed
	FName CurrentProductID;

// Variables for if this item is a trap
	/* Check for whether the item is in the process of being destroyed */
	bool bIsDestroyed = false;

	/* How much in walking speed the enemy speed is reduced, 0-1 range.  0 is completely stopped until trap is destroyed */
	float EnemySlowRate = 0.f;
	
	/* Rate of damage to the enemy per second */
	float EnemyDamageRate = 0.f;

	/* Starting amount of life for this trap if it can be destroyed */
	float StartingLife = 100.f;

	/* Time taken in game hours to repair this trap from almost destroyed to full health */
	float HoursNeededToRepair = 1.0f;

	/* Amount of damage per second this item is receiving if it is a trap */
	float CurrentDamageBeingDealt;

	/* Current amount of life this item has if it is a trap */
	float CurrentLife = 999.f;

	// Keep only in case of having a visible health bar for the trap
	// Information for enemy health bar
//	class UWidgetComponent* WidgetComp;

	/* Called on Tick to see if this item, as a trap is being damaged */
	void CheckForDamage(float DeltaTime);
	
	/* Function to destroy this item if it is a trap */
	void OnTrapDestroyed();
};
