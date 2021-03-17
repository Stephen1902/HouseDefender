// Copyright 2021 DME Games


#include "HDPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "HDAIController.h"
#include "HDEnemyMaster.h"
#include "HDWeaponMaster.h"
#include "Camera/CameraComponent.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

class AHDPlayerController;
// Sets default values
AHDPlayerCharacter::AHDPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Comp"));
	SpringArmComponent->SetupAttachment(GetMesh());
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Comp"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	

}

void AHDPlayerCharacter::TryToFire()
{
	// Check if a valid weapon is available and that the day has started
	if (CurrentWeapon != nullptr) // && bDayHasStarted)
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
	
	FVector EndLoc = (GetMesh()->GetSocketRotation(FName("FiringSocket")).Vector() * -CurrentWeapon->GetDefaultObject<AHDWeaponMaster>()->FireDistance) + StartLoc;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.bReturnPhysicalMaterial = true;

	// TODO Remove this line
		DrawDebugLine(GetWorld(), StartLoc, EndLoc, FColor::Green, true);
	
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, CollisionParams))
	{
		// Check if what was hit by the line trace was an enemy character
		if (AHDEnemyMaster* EnemyMaster = Cast<AHDEnemyMaster>(HitResult.GetActor()))
		{
			if (CurrentWeapon != nullptr)
			{
				// Check location hit by line trace to see if extra damage is required
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

	PC = Cast<APlayerController>(GetController());
	
	TimeLastFired = 0.f;
	TimeReloadStarted = 0.f;
	bDayHasStarted = false;
	
	GetTargetPoints();
}

// Called every frame
void AHDPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FMath::IsNearlyEqual(GetActorLocation().X, TPDayStart->GetActorLocation().X, 20.f) && !bDayHasStarted)
	{
		AddMovementInput(FVector(100.f * DeltaTime, 0.f, 0.f));
	}
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
	// Safety check to ensure we have a valid controller to use
	if (!PC) { return; }

	//	if (AxisValue != 0.f)
	{
		// Gets the current world location of the firing socket 
		FVector2D WeaponSocketLocation;
		PC->ProjectWorldLocationToScreen(GetMesh()->GetSocketLocation(FName("FiringSocket")),WeaponSocketLocation);
		// Check mouse position on screen
		PC->GetMousePosition(MouseX, MouseY);
		// Set current rotation for animation
		CurrentRotation = FMath::Clamp(1 - (MouseY / WeaponSocketLocation.Y), -0.5f, 0.5f);
	}
}

void AHDPlayerCharacter::GetTargetPoints()
{
	// Get all Target Points in world
	TArray<AActor*> FoundTargetPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), FoundTargetPoints);

	// Iterate through and set them to variables in this class	
	for (auto& TP : FoundTargetPoints)
	{
		ATargetPoint* ThisTP = Cast<ATargetPoint>(TP);

		if (TP->GetName() == TEXT("TPPlayerDayStart"))
		{
			TPDayStart = ThisTP;
		}

		if (TP->GetName() == TEXT("TPPlayerDayEnd"))
		{
			TPDayEnd = ThisTP;
		}

		if (TP->GetName() == TEXT("TPEnemySpawn"))
		{
			TPEnemySpawn = ThisTP;
		}
	}
	
	if (!TPDayStart || !TPDayEnd || !TPEnemySpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Target Point(s) for the player have not been set.  Please Check."));
	}
}


