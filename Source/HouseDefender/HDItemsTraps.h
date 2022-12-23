// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "HDItems.h"
#include "HDItemsTraps.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrapHit, class UUserWidget*, TrapWidget, float, NewTrapHealth);

/**
 * 
 */
UCLASS()
class HOUSEDEFENDER_API AHDItemsTraps : public AHDItems
{
	GENERATED_BODY()

	AHDItemsTraps();

protected:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps")
	bool bIsPlaceable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps", meta = (EditCondition = "bIsPlaceable"))
	float HoursNeededToPlace;


	/** Colour of the health bar, if used */
/**	UPROPERTY(EditDefaultsOnly, Category = "Traps", meta = (EditCondition = "bHasLifeAmount"))
	FLinearColor ColourOfHealthBar;

	UPROPERTY(BlueprintAssignable, Category = "Traps")
	FOnTrapHit OnTrapHit;

	UFUNCTION(BlueprintCallable, Category = "Trap Functions")
	FLinearColor GetColourOfHealthBar() const { return ColourOfHealthBar; }
*/
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void SetCurrentDamage(const float DamageIn) { CurrentDamageBeingDealt += DamageIn; }

	void SetStartingVariables(bool SlowsEnemyIn, float SlowRateIn, bool DamagesEnemyIn, float EnemyDamageRateIn, bool HasLifeAmountIn, float StartingLifeAmountIn, float HoursToRepairIn);
private:
	float CurrentLife;
	float CurrentDamageBeingDealt;

	bool bIsBeingOverlapped;
	bool bHasBeenDestroyed;

	void CheckForDamage(float DeltaTime);
	void SetWidgetInfo();
	FVector WidgetLocationAtStart;

	UPROPERTY()
	class UUserWidget* UserWidget;

	FTimerHandle FakeTimer;
	void OnTrapDestroyed();

	TArray<class AHDEnemyMaster*> OverlappingEnemies;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps")
	bool bSlowsEnemy = false;
/** Keep only in case of having a visible health widget 
	// Information for enemy health bar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Enemy Pawn")
	class UWidgetComponent* WidgetComp;
*/
	/** How much in walking speed the enemy speed is reduced, 0-1 range.  0 is completely stopped until trap is destroyed */
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps", meta = (EditCondition = "bSlowsEnemy", ClampMax = 1))
	float EnemySlowRate = 0.f;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps")
	bool bDamagesEnemy = false;

	/** Rate of damage to the enemy per second */
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps", meta = (EditCondition = "bDamagesEnemy"))
	float EnemyDamageRate = 0.f;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps")
	bool bHasLifeAmount = true;

	/** Starting amount of life for this trap */
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps", meta = (EditCondition = "bHasLifeAmount"))
	float StartingLife = 100.f;

	/** Time taken to repair this trap from almost destroyed to full health */
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traps", meta = (EditCondition = "bIsPlaceable"))
	float HoursNeededToRepair = 1.0f;

};