// Copyright 2021 DME Games


#include "HDPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "HDAIController.h"
#include "HDEnemyMaster.h"
#include "HDWeaponMaster.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AHDPlayerCharacter::AHDPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Comp"));
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Comp"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	

}

void AHDPlayerCharacter::TryToFire()
{
	// Check if a valid weapon is available
	if (CurrentWeapon != nullptr)
	{
		// Check enough time has passed since last firing against the weapon's fire rate
		if (TimeLastFired < GetWorld()->GetTimeSeconds() - CurrentWeapon->GetDefaultObject<AHDWeaponMaster>()->FireRate)
		{
			// Check the weapon has ammo
//			if (CurrentWeapon->GetDefaultObject<AHDWeaponMaster>()->CurrentAmmoInWeapon > 0)
			{
				// Call the fire function
				Fire();
			}
//			else
			{
				//		TryToReload();
			}
		}
	}
}

void AHDPlayerCharacter::Fire()
{
	TimeLastFired = GetWorld()->GetTimeSeconds();

	FHitResult HitResult;
	const FVector StartLoc = GetMesh()->GetSocketLocation(FName("FiringSocket"));
	
	FVector EndLoc = StartLoc;

	const float CurrentRotationAsPercentage = 1 - (CurrentRotation * 2);
	const float FireDistancePositive = CurrentWeapon->GetDefaultObject<AHDWeaponMaster>()->FireDistance;
	const float FireDistanceNegative = -CurrentWeapon->GetDefaultObject<AHDWeaponMaster>()->FireDistance;

	// Get Z Axis as percentage of total rotation
	EndLoc.Z = FMath::Clamp(((1 - CurrentRotationAsPercentage) * FireDistancePositive) + StartLoc.Z, FireDistanceNegative, FireDistancePositive);
	if (EndLoc.Z < 0.f)
	{
		EndLoc.X = FMath::Clamp(EndLoc.Z + FireDistancePositive, FireDistanceNegative, FireDistancePositive);
	}
	else
	{
		EndLoc.X = FMath::Clamp(FireDistancePositive - EndLoc.Z, FireDistanceNegative, FireDistancePositive);
	}
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.bReturnPhysicalMaterial = true;
	// CollisionParams.bTraceComplex = true;

		DrawDebugLine(GetWorld(), StartLoc, EndLoc, FColor::Green, true);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, CollisionParams))
	{

		if (AHDEnemyMaster* EnemyMaster = Cast<AHDEnemyMaster>(HitResult.GetActor()))
		{
			if (CurrentWeapon != nullptr)
			{
				EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

				float DamageMultiplier = 1.f;
			
				switch (SurfaceType)
				{
				case SURFACE_FleshStandard:
				default:
					DamageMultiplier = 1.f;
					break;
				case SURFACE_FleshVulnerable:
					DamageMultiplier = CurrentWeapon->GetDefaultObject<AHDWeaponMaster>()->HeadshotBonus;
					break;
				}

				EnemyMaster->SetCurrentLife(CurrentWeapon->GetDefaultObject<AHDWeaponMaster>()->DamagePerShot * DamageMultiplier);
			}			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("I hit something that wasn't an enemy"));
		}		
	}
}

// Called when the game starts or when spawned
void AHDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CurrentWeapon == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon not set in blueprint.  Game cannot proceed."));
	}

	TimeLastFired = 0.f;
	TimeReloadStarted = 0.f;
}

// Called every frame
void AHDPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AHDPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("LookUp", this, &AHDPlayerCharacter::Look);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHDPlayerCharacter::TryToFire);
}

void AHDPlayerCharacter::Look(float AxisValue)
{
//	if (AxisValue != 0.f)
	{

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			// Gets the current world location of the firing socket 
			FVector2D WeaponSocketLocation;
			PC->ProjectWorldLocationToScreen(GetMesh()->GetSocketLocation(FName("FiringSocket")),WeaponSocketLocation);

			PC->GetMousePosition(MouseX, MouseY);
			
			CurrentRotation = FMath::Clamp(1 - (MouseY / WeaponSocketLocation.Y), -0.5f, 0.5f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No controller found on this player"));
		}
		
	}
}

