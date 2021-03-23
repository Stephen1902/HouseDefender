// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"

#include <activation.h>

#include "GameFramework/Character.h"
#include "HDPlayerCharacter.generated.h"

#define SURFACE_FleshStandard		SurfaceType1
#define SURFACE_FleshVulnerable		SurfaceType2
#define SURFACE_FleshArmoured		SurfaceType3

UENUM(BlueprintType)
enum class ECurrentWeapon : uint8
{
	CW_None			UMETA(DisplayName = "No Weapon"),
	CW_Pistol		UMETA(DisplayName = "Pistol"),
	CW_Shotgun		UMETA(DisplayName = "Shotgun"),
	CW_Rifle		UMETA(DisplayName = "Rifle")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChanged, ECurrentWeapon, NewWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReload, float, ReloadTimeIn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoUpdated, FText, NewAmmoInClip, FText, NewAmmoAvailable);
DECLARE_DELEGATE_OneParam(FKeyboardWeaponSelect, int32);

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

	// Widget to show reload status
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Gameplay")
	class UWidgetComponent* ReloadWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TArray<TSubclassOf<class AHDWeaponMaster>> CurrentWeapon;

	UFUNCTION(BlueprintCallable, Category="Gameplay")
	float GetCurrentRotation() const { return CurrentRotation; }

	UPROPERTY(EditAnywhere, Category = "Character Weapon")
	ECurrentWeapon CurrentPlayerWeapon;

	UFUNCTION(BlueprintCallable, Category = "Character Weapon")
	ECurrentWeapon GetCurrentPlayerWeapon() { return CurrentPlayerWeapon; }

	UPROPERTY(BlueprintAssignable, Category = "Character Weapon")
	FOnAmmoUpdated OnAmmoUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Character Weapon")
	FOnReload OnReload;
	
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
	void WeaponSelected(int32 WeaponSelectedIn);
	void ManualReload();
	void BeginReload();
	void Reload();
	void UpdateAmmo();
	void CheckCurrentGameState(float DeltaTime);
	void CheckForReloadStatus();
	
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

	// Current game weapon
	AHDWeaponMaster* WeaponToSpawn = nullptr;

	// Current Enum Index
	uint8 CurrentEnumIndex;

	// Timer Handles
	FTimerHandle ReloadTimer;
};



