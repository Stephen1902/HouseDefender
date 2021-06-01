// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HDPlayerCharacter.generated.h"

#define SURFACE_FleshStandard		SurfaceType1
#define SURFACE_FleshVulnerable		SurfaceType2
#define SURFACE_FleshArmoured		SurfaceType3

USTRUCT(BlueprintType)
struct FWeaponInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FText WeaponName;
	
	UPROPERTY()
	float DamagePerShot;

	UPROPERTY()
	float VulnerableBonus;

	UPROPERTY()
	float ArmourPenalty;

	UPROPERTY()
	float FireRate;

	UPROPERTY()
	float FireDistance;

	UPROPERTY()
	int32 MagazineSize;

	UPROPERTY()
	int32 TotalAmmo;

	UPROPERTY()
	int32 CurrentAmmoInClip;

	UPROPERTY()
	float ReloadTime;

	UPROPERTY()
	int32 WeaponType;

	UPROPERTY()
	bool bPlayerHasThisWeapon;

	FWeaponInfo()
	{
		WeaponName = FText::FromString(TEXT(""));
		DamagePerShot = 0.0f;
		VulnerableBonus = 0.0f;
		ArmourPenalty = 0.0f;
		FireRate = 0.0f;
		FireDistance = 0.0f;
		MagazineSize = 0;
		TotalAmmo = 0;
		CurrentAmmoInClip = 0;
		ReloadTime = 0.0f;
		WeaponType = 1;
		bPlayerHasThisWeapon = false;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChanged, int32, NewWeaponValue);
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

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* GamePlaySpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* GamePlayCameraComp;

	UPROPERTY(VisibleAnywhere, Category = "Player")
	class UHDInventoryComponent* PlayerInventory;

	// Widget to show reload status
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "Gameplay")
	class UWidgetComponent* ReloadWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TArray<TSubclassOf<class AHDWeaponMaster>> WeaponList;

	UFUNCTION(BlueprintCallable, Category="Gameplay")
	float GetCurrentRotation() const { return CurrentRotation; }

	UFUNCTION(BlueprintCallable, Category = "Character Weapon")
	int32 GetCurrentWeaponIndex() const;

	UFUNCTION(BlueprintCallable, Category = "Character Weapon")
	TArray<TSubclassOf<class AHDWeaponMaster>> GetAvailableWeapons() const { return WeaponList; }
	
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
	void LookUp(float AxisValue);
	void LookRight(float AxisValue);
	void GetTargetPoints();
	void SetDayStartCameraLocation();
	void MovePlayerToDayStartPosition(float DeltaTime);
	void MovePlayerToDayEndPosition(float DeltaTime);
	void CheckForLivingEnemies() const;
	void MoveCameraToNewLocation(AActor* ActorToMoveTo) const;
	void WeaponSelected(int32 WeaponSelectedIn);
	void ManualReload();
	void BeginReload();
	void Reload();
	void UpdateAmmo();
	void CheckCurrentGameState(float DeltaTime);
	void CheckForReloadStatus();
	void AddWeaponInfo();
	
	// Current player rotation, allowing them to look up or down based on value
	float CurrentRotation;

	float TimeLastFired;
	float TimeReloadStarted;

	// Mouse Co-ordinates to calculate rotation of player and firing location
	float MouseX;
	float MouseY;

	// Target points for the player to move to at the start / end of the day
	UPROPERTY()
	class ATargetPoint* TPDayStart;
	
	UPROPERTY()
	class ATargetPoint* TPDayEnd;

	UPROPERTY()
	class ATargetPoint* TPEnemySpawn;

	UPROPERTY()
	class AActor* DayViewLocation;

	float MidPointBetweenDayStartAndEnemySpawn = 0.f;

	// Player Controller
	UPROPERTY()
	class APlayerController* PC;

	// Game State
	UPROPERTY()
	class AHDGameStateBase* GSBase;

	// Current game weapon
	UPROPERTY()
	AHDWeaponMaster* WeaponToSpawn = nullptr;

	// Current Enum Index
	int32 CurrentWeaponIndex;

	// Stored weapon index at end of day
	int32 EndOfDayWeaponIndex;

	// Timer Handles
	FTimerHandle ReloadTimer;

	// Weapon struct
	FWeaponInfo WeaponInfoStruct;
	
	// Array of all weapons for the player
	TArray<FWeaponInfo> WeaponInfoArray;
};



