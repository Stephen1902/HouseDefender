// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HDPlayerCharacter.generated.h"

#define SURFACE_FleshStandard		SurfaceType1
#define SURFACE_FleshVulnerable		SurfaceType2
#define SURFACE_FleshArmoured		SurfaceType3

UCLASS()
class HOUSEDEFENDER_API AHDPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHDPlayerCharacter();

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* CameraComp;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TSubclassOf<class AHDWeaponMaster> CurrentWeapon;

	UFUNCTION(BlueprintCallable, Category="Gameplay")
	float GetCurrentRotation() const { return CurrentRotation; }

protected:
	void TryToFire();
	void Fire();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void Look(float AxisValue);
	void GetTargetPoints();
	void SetDayStartCameraLocation();
	void MovePlayerToDayStartPosition(float DeltaTime);
	void MovePlayerToDayEndPosition(float DeltaTime);
	void CheckForLivingEnemies();
	void MoveCameraToNewLocation(AActor* ActorToMoveTo) const;

	// Current player rotation, allowing them to look up or down based on value
	float CurrentRotation;

	float TimeLastFired;
	float TimeReloadStarted;

	// Mouse Co-ordinates to calculate rotation of player and firing location
	float MouseX;
	float MouseY;

	// Target points for the player to move to at the start / end of the day
	class ATargetPoint* TPDayStart;
	class ATargetPoint* TPDayEnd;
	class ATargetPoint* TPEnemySpawn;
	class AActor* DayViewLocation;

	float MidPointBetweenDayStartAndEnemySpawn = 0.f;

	// Player Controller
	class APlayerController* PC;

	// Game State
	class AHDGameStateBase* GSBase;
};
