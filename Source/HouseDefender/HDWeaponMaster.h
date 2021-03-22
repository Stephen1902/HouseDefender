// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HDWeaponMaster.generated.h"

UCLASS()
class HOUSEDEFENDER_API AHDWeaponMaster : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHDWeaponMaster();

	UPROPERTY(VisibleAnywhere, Category = "SetUp")
	class USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "SetUp")
	FText WeaponName;

	// Base damage per shot
	UPROPERTY(EditDefaultsOnly, Category = "SetUp")
	float DamagePerShot;

	// Damage multiplier when hitting an area of vulnerable skin ie headshot
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 1.0f), Category = "SetUp")
	float VulnerableBonus;

	// Damage penalty when hitting an area of armoured skin
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.0f, ClampMax = 1.0f), Category = "SetUp")
	float ArmourPenalty;

	UPROPERTY(EditDefaultsOnly, Category = "SetUp")
	float FireRate;

	UPROPERTY(EditDefaultsOnly, Category = "SetUp")
	float FireDistance;
	
	UPROPERTY(EditDefaultsOnly, Category = "SetUp")
	int32 MagazineSize;

	UPROPERTY(EditDefaultsOnly, Category = "SetUp")
	float ReloadTime;
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	int32 CurrentAmmoInWeapon;
	int32 CurrentAmmoForWeapon;
};
