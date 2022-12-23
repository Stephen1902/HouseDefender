// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HD_BlueprintFunctionLibrary.generated.h"


class AHDItems;
/**
 * A class for putting items that need to be available easily for both C++ and Blueprint.
 * Specifically, game items that can be picked up or need to be crafted.
 */

UENUM(BlueprintType)
enum class EItemType : uint8
{
	IT_None			UMETA(DisplayName = "None"),
	IT_Plants		UMETA(DisplayName = "Medicinal Part"),
	IT_TrapPart		UMETA(DisplayName = "Trap Part"),
	IT_AmmoPart		UMETA(DisplayName = "Ammo Part"),
	IT_Medicinal	UMETA(DisplayName = "Medicinal"),
	IT_Trap			UMETA(DisplayName = "Trap"),
	IT_Ammo			UMETA(DisplayName = "Ammo")
};

USTRUCT(BlueprintType)
struct FTrapInfo
{
	GENERATED_BODY()
/* Keep for when adding to constraints
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps")
	float HoursNeededToPlace;

	// Time taken to repair this trap from almost destroyed to full health
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps")
	float HoursNeededToRepair;
*/
/** Keep only in case of having a visible health widget 
	// Information for enemy health bar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Enemy Pawn")
	class UWidgetComponent* WidgetComp;
*/
	/** How much in walking speed the enemy speed is reduced, 0-1 range.  0 is completely stopped until trap is destroyed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps", meta = (ClampMax = 1))
	float EnemySlowRate = 0.f;
	
	/** Rate of damage to the enemy per second */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps", meta = (EditCondition = "bDamagesEnemy"))
	float EnemyDamageRate = 0.f;

	/** Starting amount of life for this trap */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps", meta = (EditCondition = "bHasLifeAmount"))
	float StartingLife = 100.f;

	/* Game hours needed to return this item from 1 health to maximum */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps")
	float HoursNeededToRepair = 1.f;

	// An optional sound that is played when this trap is interacted with
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	USoundBase* SoundForInteraction;

	// An optional sound that is played when the trap is destroyed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	USoundBase* SoundForDestroyed;

	// An optional particle effect to be played when the trap is destroyed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	UParticleSystem* ParticleForDestroyed;
	
	/** Colour of the health bar, if used */
/**	UPROPERTY(EditDefaultsOnly, Category = "Traps", meta = (EditCondition = "bHasLifeAmount"))
	FLinearColor ColourOfHealthBar;

	UPROPERTY(BlueprintAssignable, Category = "Traps")
	FOnTrapHit OnTrapHit;

	UFUNCTION(BlueprintCallable, Category = "Trap Functions")
	FLinearColor GetColourOfHealthBar() const { return ColourOfHealthBar; }*/
};

// Table structure that can be used to create a blueprint of a table which can be exported to CSV file types to fill in externally
USTRUCT(BlueprintType)
struct FItemInformationTable : public FTableRowBase
{
	GENERATED_BODY()

	// A unique identifier for this item.  Used to help prevent errors in selecting from the data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	FName ItemId;

	// What type of item this is
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	EItemType ItemType;

	// The name that will appear on screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	FText ItemName;

	// A short desription of the item to appear in the player inventory / crafting screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	FText ItemDescription;

	// Mesh to display when shown on screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	class UStaticMesh* ItemDisplayMesh;

	// Image to display in the player inventory / crafting screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	class UTexture2D* ItemDisplayTexture;

	// Will a trader buy this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	bool bCanBeTraded;

	// How much the trader will pay for this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items", meta = (EditCondition = "bCanBeTraded"))
	float ItemValueSold;

	// How much the player will have to pay for this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items", meta = (EditCondition = "bCanBeTraded"))
	float ItemValueBought;

	// Whether or not this item is crafted from parts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items")
	bool bIsCrafted;

	// Parts needed to craft this item, first item is the ItemId, second is the quantity required of the item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items", meta = (EditCondition = "bIsCrafted"))
	TMap<FName, int32> NeededForCrafting;

	// Item information if this item is a trap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Items", meta = (EditCondition = "ItemType == EItemType::IT_Trap"))
	FTrapInfo TrapInfo;
};

// A struct used to pass information from the Ghost Building to other classes
USTRUCT(BlueprintType)
struct FConstructionStruct
{
	GENERATED_BODY();

	UPROPERTY()
	FName BuildingID;

	UPROPERTY()
	class AHDConstructionProxy* ProxyClassRef;

	UPROPERTY()
	FVector GhostBuildingLocation;

	UPROPERTY()
	FRotator GhostBuildingRotation;

	UPROPERTY()
	UStaticMesh* DisplayMesh;
};

UCLASS()
class HOUSEDEFENDER_API UHD_BlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tests")
	void TestFunction();
	
};
