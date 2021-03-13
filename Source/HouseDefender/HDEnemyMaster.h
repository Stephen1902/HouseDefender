// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"

#include "HDPlayerCharacter.h"
#include "GameFramework/Pawn.h"
#include "HDEnemyMaster.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyHit, class UUserWidget*, EnemyPawnWidget, float, NewEnemyHealth);

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

	UPROPERTY(BlueprintAssignable, Category = "Enemy Pawn")
	FOnEnemyHit OnEnemyHit;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Enemy Functions")
    bool GetHasBeenHit() const { return bHasBeenHit; }

	UFUNCTION(BlueprintCallable, Category = "Enemy Functions")
    void SetHasBeenHit(bool NewHitIn);

	UFUNCTION(BlueprintCallable, Category = "Enemy Functions")
	bool GetIsDead() const;

	void SetCurrentLife(float LifeTakenOff);

private:
	bool bHasBeenHit = false;

	float CurrentLife;

	FVector WidgetLocationAtStart;

	FTimerHandle TimerHandle_EndOfLife;

	void DestroyEnemy();

	FTimerHandle TimerHandle_HasBeenHit;
	void ClearHasBeenHit();

	void MoveTowardsPlayer(float DeltaTime);

	class AHDPlayerCharacter* PlayerCharacter;

	void GetPlayerCharacter();
	void UpdateWidget();
};
