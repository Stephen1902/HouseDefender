// Copyright 2021 DME Games


#include "HDPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "HDAIController.h"
#include "HDEnemyMaster.h"
#include "HDGameStateBase.h"
#include "HDWeaponMaster.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

class AHDPlayerController;
// Sets default values
AHDPlayerCharacter::AHDPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComponent->SetupAttachment(GetMesh());
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComponent);
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
				float ArmourPenalty = 1.f;
			
				switch (SurfaceType)
				{
				case SURFACE_FleshStandard:
				default:
					DamageMultiplier = 1.f;
					break;
				case SURFACE_FleshVulnerable:
					DamageMultiplier = CurrentWeapon->GetDefaultObject<AHDWeaponMaster>()->VulnerableBonus;
					break;
				case SURFACE_FleshArmoured:
					ArmourPenalty = CurrentWeapon->GetDefaultObject<AHDWeaponMaster>()->ArmourPenalty;
					break;
				}

				EnemyMaster->SetCurrentLife(CurrentWeapon->GetDefaultObject<AHDWeaponMaster>()->DamagePerShot * DamageMultiplier * ArmourPenalty);
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
	GSBase = Cast<AHDGameStateBase>(GetWorld()->GetGameState());

	if (GSBase)
	{
		GSBase->SetGameStatus(EGameStatus::GS_DayStarting);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Game State Base has not been set."));
	}
	
	TimeLastFired = 0.f;
	TimeReloadStarted = 0.f;
	
	GetTargetPoints();
	//SetDayStartCameraLocation();
}

// Called every frame
void AHDPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GSBase)
	{
		if (GSBase->GetGameStatus() == EGameStatus::GS_DayStarting)
		{
			MovePlayerToDayStartPosition(DeltaTime);
		}

		if (GSBase->GetGameStatus() == EGameStatus::GS_DayStarted)
		{
			CheckForLivingEnemies();
		}

		if (GSBase->GetGameStatus() == EGameStatus::GS_DayEnding)
		{
			MovePlayerToDayEndPosition(DeltaTime);
		}
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

		if (TP->GetName() == TEXT("TPPlayerViewLocation"))
		{
			DayViewLocation = TP;
		}
	}

	if (!TPDayStart || !TPDayEnd || !TPEnemySpawn || !DayViewLocation)
	{
		UE_LOG(LogTemp, Warning, TEXT("Target Point(s) for the player have not been set.  Please Check."));
	}
}

void AHDPlayerCharacter::SetDayStartCameraLocation()
{
	FVector CurrentCameraLocation = DayViewLocation->GetActorLocation();

	// Get the midway point between the player location at day start and enemy spawn point
	MidPointBetweenDayStartAndEnemySpawn = (TPDayStart->GetDistanceTo(TPEnemySpawn)) / 2;

	// Set Y location to the midpoint between the 2 less 400 (4 metres) so enemies do not spawn on screen
	CurrentCameraLocation.X = (MidPointBetweenDayStartAndEnemySpawn + 400.f); 
	
	DayViewLocation->SetActorLocation(CurrentCameraLocation);

	// TODO Set up proper game start status to move player and spawn enemies
	MoveCameraToNewLocation(DayViewLocation);
}

void AHDPlayerCharacter::MovePlayerToDayStartPosition(float DeltaTime)
{
	if (!FMath::IsNearlyEqual(GetActorLocation().X, TPDayStart->GetActorLocation().X, 20.f))
	{
		AddMovementInput(FVector(100.f * DeltaTime, 0.f, 0.f));
		
		if (!FMath::IsNearlyEqual(GetMesh()->GetRelativeRotation().Yaw, -90.f, 20.f))
		{
			const float GetCurrentYawRotation = GetMesh()->GetRelativeRotation().Yaw;
			const float NewYaw = GetCurrentYawRotation + (120.f * DeltaTime);
			GetMesh()->SetRelativeRotation(FRotator(0.f, NewYaw, 0.f));
		}
	}
	else
	{
		if (GSBase->GameStatus != EGameStatus::GS_DayStarted)
		{
			SetDayStartCameraLocation();		
			GSBase->SetGameStatus(EGameStatus::GS_DayStarted);
		}
	}
}

void AHDPlayerCharacter::MovePlayerToDayEndPosition(float DeltaTime)
{
	if (!FMath::IsNearlyEqual(GetActorLocation().X, TPDayEnd->GetActorLocation().X, 20.f))
	{
		AddMovementInput(FVector(-100.f * DeltaTime, 0.f, 0.f));

		if (!FMath::IsNearlyEqual(GetMesh()->GetRelativeRotation().Yaw, -270.f, 20.f))
		{
			const float GetCurrentYawRotation = GetMesh()->GetRelativeRotation().Yaw;
			const float NewYaw = GetCurrentYawRotation - (120.f * DeltaTime);
			GetMesh()->SetRelativeRotation(FRotator(0.f, NewYaw, 0.f));
		}
	}
	else
	{
		if (GSBase->GameStatus != EGameStatus::GS_Idle)
		{
			GetMesh()->SetRelativeRotation(FRotator(0.f, -270.f, 0.f));
			GSBase->SetGameStatus(EGameStatus::GS_Idle);
		}
	}
}

void AHDPlayerCharacter::CheckForLivingEnemies()
{
	if (GSBase->GameStatus == EGameStatus::GS_DayStarted)
	{
		TArray<AActor*> FoundEnemies;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHDEnemyMaster::StaticClass(), FoundEnemies);

		if (FoundEnemies.Num() == 0)
		{
			MoveCameraToNewLocation(this);
			GSBase->SetGameStatus(EGameStatus::GS_DayEnding);
		}
	}
}

void AHDPlayerCharacter::MoveCameraToNewLocation(AActor* ActorToMoveTo) const
{
	// Set the camera to a new position, depending on game state 
	PC->SetViewTargetWithBlend(ActorToMoveTo, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
}



