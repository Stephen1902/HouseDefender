// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "HDItems.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	IT_Plants		UMETA(DisplayName = "Plants"),
	IT_TrapPart		UMETA(DisplayName = "Trap Part"),
	IT_AmmoPart		UMETA(DisplayName = "Ammo Part"),
	IT_Medicinal	UMETA(DisplayName = "Medicinal"),
	IT_Trap			UMETA(DisplayName = "Trap"),
	IT_Ammo			UMETA(DisplayName = "Ammo")
};

UCLASS()
class HOUSEDEFENDER_API AHDItems : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHDItems();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	class UStaticMeshComponent* ItemMesh;

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
	EItemType ItemType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
	bool bIsCrafted;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items", meta = (EditCondition = "bIsCrafted"))
	TMap<TSubclassOf<AHDItems>, int32> NeededForCrafting; 

	UPROPERTY()
	UTimelineComponent* MyTimeline;

	void SetNumberOfItemsToAdd(int32 NumberToAddIn) { NumberOfItemsToAdd = NumberToAddIn; }
	
protected:
	// World item to be added to the inventory when this pick up is collected
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "World Item")
	AHDItems* DroppedItem;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void TimelineFloatReturn(float Value);

	UFUNCTION()
	void OnTimelineFinished();

	/** Declare a delegate to call with TimeLineFloatReturn */
	FOnTimelineFloat InterpFunction{};

	/** Declare a delegate to call with OnTimelineFinished */
	FOnTimelineEvent TimelineFinished{};


private:
	UPROPERTY()
	class AHDGameStateBase* GameStateBaseRef;

	UPROPERTY()
	class AHDPlayerCharacter* PlayerCharacter;

	UFUNCTION()
	void GameStateChanged();

	FVector DropStartLocation;
	FVector DropEndLocation;

	//    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timeline")
	/** Curve to be used for moving dropped items to the corner of the screen */
	UPROPERTY()
	class UCurveFloat* FCurve;
	
	float MaxCurveTimeRange;

	void GetMovementCurve();
	void GetReferences();
	void SetDropLocations();

	int32 NumberOfItemsToAdd;
};
