// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "HDPlayerCharacter.h"
#include "GameFramework/Pawn.h"
#include "HDEnemyMaster.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyHit, class UUserWidget*, EnemyPawnWidget, float, NewEnemyHealth);

USTRUCT(BlueprintType)
struct FDroppableItems
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Droppable Item")
	TSubclassOf<class AHDItems> ItemToDrop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Droppable Item")
	float DropProbability;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Droppable Item")
	int32 MaxDropAmount;

	FDroppableItems()
	{
		ItemToDrop = nullptr;
		DropProbability = 0.5f;
		MaxDropAmount = 1;
	}
};

UCLASS()
class HOUSEDEFENDER_API AHDEnemyMaster : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHDEnemyMaster();

	UPROPERTY(VisibleAnywhere, Category = "Enemy Pawn")
	class UCapsuleComponent* CapsuleComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Enemy Pawn")
	class USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Enemy Pawn")
	class UPawnMovementComponent* MovementComponent;

	// Information for enemy health bar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Enemy Pawn")
	class UWidgetComponent* WidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy Pawn")
	float StartingLife;

	// distance covered in seconds
	UPROPERTY(EditDefaultsOnly, Category = "Enemy Pawn")
	float MovementSpeed;

	// Damage dealt to traps per second 
	UPROPERTY(EditDefaultsOnly, Category = "Enemy Pawn")
	float DamageDealtToTraps;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy Pawn")
	FLinearColor ColourOfHealthBar;
	
	UPROPERTY(BlueprintAssignable, Category = "Enemy Pawn")
	FOnEnemyHit OnEnemyHit;

	/** Items that this type of enemy can drop */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Droppable Item")
	TArray<FDroppableItems> DroppableItemsList;

	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

/** Kept here in case of later use
	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
*/
	
	UFUNCTION(BlueprintCallable, Category = "Enemy Functions")
    bool GetHasBeenHit() const { return bHasBeenHit; }

	UFUNCTION(BlueprintCallable, Category = "Enemy Functions")
    void SetHasBeenHit(bool NewHitIn);

	UFUNCTION(BlueprintCallable, Category = "Enemy Functions")
	bool GetIsBlocked() const { return bIsBlocked; }

	// Set whether or not the enemy is blocked by a trap
	UFUNCTION(BlueprintCallable, Category = "Enemy Functions")
	void SetIsBlocked(const bool NewBlockIn) { bIsBlocked = NewBlockIn; }

	UFUNCTION(BlueprintCallable, Category = "Enemy Functions")
	bool GetIsDead() const;

	UFUNCTION(BlueprintCallable, Category = "Enemy Functions")
	FLinearColor GetColourOfHealthBar() const { return ColourOfHealthBar; }

	void SetCurrentLife(float LifeTakenOff);

	void SetSpeedReductionFromTraps(const float SpeedIn) { SpeedReductionFromTraps = SpeedIn; }
	void SetDamageTaken(const float DamageIn) { DamageTaken = DamageIn; }

	float GetDamageDealtToTraps() const { return DamageDealtToTraps; }

private:
	bool bHasBeenHit = false;
	bool bIsBlocked = false;

	float CurrentLife;


	FTimerHandle TimerHandle_EndOfLife;

	void DestroyEnemy();

	FTimerHandle TimerHandle_HasBeenHit;
	void ClearHasBeenHit();

	void MoveTowardsPlayer(float DeltaTime);

	void CheckForDamage(float DeltaTime);

	UPROPERTY()
	AActor* StairsToReach;

	UPROPERTY()
	class AHDPlayerCharacter* PlayerCharacter;

	void GetPlayerCharacter();
	void UpdateWidgetLocation() const;
	void UpdateWidgetInformation() const;
	void SetWidgetInfo();

	float SpeedReductionFromTraps;
	float DamageTaken;

	FVector WidgetLocationAtStart;

	UPROPERTY()
	class UUserWidget* UserWidget;
	
};