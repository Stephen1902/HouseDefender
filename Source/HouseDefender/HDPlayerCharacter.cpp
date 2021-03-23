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

	ReloadWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	ReloadWidgetComp->SetupAttachment(GetMesh());
	ReloadWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	ReloadWidgetComp->SetDrawSize(FVector2D(64.f, 64.f));
	ReloadWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	ReloadWidgetComp->SetHiddenInGame(true);
}

void AHDPlayerCharacter::TryToFire()
{

	// Check if a valid weapon is available and that the day has started
	if (CurrentWeapon.Num() > 0 && CurrentEnumIndex > 0)
	{
		// Check enough time has passed since last firing against the weapon's fire rate
		if (TimeLastFired < GetWorld()->GetTimeSeconds() - CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->FireRate)
		{
			// Check the weapon has ammo and they are not reloading
			if (CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->CurrentAmmoInClip > 0 && !GetWorld()->GetTimerManager().IsTimerActive(ReloadTimer))
			{
				// Call the fire function
				Fire();
			}
			else
			{
				// Check there is ammo available to reload into the gun
				if (CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->TotalAmmo > 0)
				{
					BeginReload();
				}
			}
		}
	}
}

void AHDPlayerCharacter::Fire()
{
	TimeLastFired = GetWorld()->GetTimeSeconds();

	CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->CurrentAmmoInClip--;
	UpdateAmmo();
	
	FHitResult HitResult;
	const FVector StartLoc = GetMesh()->GetSocketLocation(FName("FiringSocket"));
	
	FVector EndLoc = (GetMesh()->GetSocketRotation(FName("FiringSocket")).Vector() * - CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->FireDistance) + StartLoc;

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
				DamageMultiplier = CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->VulnerableBonus;
				break;
			case SURFACE_FleshArmoured:
				ArmourPenalty = CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->ArmourPenalty;
				break;
			}

			EnemyMaster->SetCurrentLife(CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->DamagePerShot * DamageMultiplier * ArmourPenalty);			
		}
	}
}

// Called when the game starts or when spawned
void AHDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CurrentWeapon.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon not set in blueprint.  Game cannot proceed."));
	}
	else
	{
		
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
	WeaponSelected(1);
	UpdateAmmo();
}

// Called every frame
void AHDPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckCurrentGameState(DeltaTime);
	CheckForReloadStatus();
	
}

// Called to bind functionality to input
void AHDPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("LookUp", this, &AHDPlayerCharacter::Look);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHDPlayerCharacter::TryToFire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AHDPlayerCharacter::ManualReload);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot1", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 1);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot2", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 2);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot3", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 3);
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

void AHDPlayerCharacter::WeaponSelected(int32 WeaponSelectedIn)
{
	if (CurrentEnumIndex != static_cast<uint8>(WeaponSelectedIn))
	{
		if (WeaponToSpawn)
		{
			WeaponToSpawn->Destroy();
		}

		// Check player wasn't mid reload when changing weapon, cancel reload if they were
		if (GetWorld()->GetTimerManager().IsTimerActive(ReloadTimer))
		{
			GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
		}
		
		const FActorSpawnParameters SpawnParameters;
		WeaponToSpawn = GetWorld()->SpawnActor<AHDWeaponMaster>(CurrentWeapon[WeaponSelectedIn], GetActorLocation(), GetActorRotation(), SpawnParameters);
		const FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
		WeaponToSpawn->AttachToComponent(GetMesh(), TransformRules, FName("WeaponSocket"));

		switch(WeaponSelectedIn)
		{
			case 0:
			default:
				CurrentPlayerWeapon = ECurrentWeapon::CW_None;
				break;
			case 1:
				CurrentPlayerWeapon = ECurrentWeapon::CW_Pistol;
				break;
			case 2:
				CurrentPlayerWeapon = ECurrentWeapon::CW_Shotgun;
				break;
			case 3:
				CurrentPlayerWeapon = ECurrentWeapon::CW_Rifle;
				break;
		}
		
		CurrentEnumIndex = static_cast<uint8>(WeaponSelectedIn);

		UpdateAmmo();
	}
}

void AHDPlayerCharacter::ManualReload()
{
	if (CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->TotalAmmo > 0)
	{
		// Add bullets in clip to available bullets
		CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->TotalAmmo += CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->CurrentAmmoInClip;
	
		// Empty the bullets in the clip
		CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->CurrentAmmoInClip = 0;

		// Reload the weapon
		BeginReload();
	}
}

void AHDPlayerCharacter::BeginReload()
{
	// Prevent spamming of the reload key
	if (!GetWorld()->GetTimerManager().IsTimerActive(ReloadTimer))
	{
		const float ReloadTimerTime = CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->ReloadTime;
		
		GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &AHDPlayerCharacter::Reload, ReloadTimerTime, false);
	}
}

void AHDPlayerCharacter::Reload()
{
	const int32 AmmoToAmend = FMath::Min(CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->MagazineSize, CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->TotalAmmo);
	CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->CurrentAmmoInClip += AmmoToAmend;
	CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->TotalAmmo -= AmmoToAmend;

	UpdateAmmo();
	ReloadWidgetComp->SetHiddenInGame(true);
}

void AHDPlayerCharacter::UpdateAmmo()
{
	FText NewAmmoInClip = FText::FromString(FString::FromInt(0));
	FText NewAmmoAvailable = FText::FromString(FString::FromInt(0));
	
	if (CurrentEnumIndex > 0)
	{
		NewAmmoInClip = FText::FromString(FString::FromInt(CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->CurrentAmmoInClip));
		NewAmmoAvailable = FText::FromString(FString::FromInt(CurrentWeapon[CurrentEnumIndex]->GetDefaultObject<AHDWeaponMaster>()->TotalAmmo));
	}

	OnAmmoUpdated.Broadcast(NewAmmoInClip, NewAmmoAvailable);
}

void AHDPlayerCharacter::CheckCurrentGameState(float DeltaTime)
{
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

void AHDPlayerCharacter::CheckForReloadStatus()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ReloadTimer))
	{
		if (!ReloadWidgetComp->IsWidgetVisible())
		{
			ReloadWidgetComp->SetHiddenInGame(false);
		}
		
		const float TimeRemainingAsPercentage = ((CurrentWeapon[CurrentEnumIndex].GetDefaultObject()->ReloadTime - GetWorld()->GetTimerManager().GetTimerRemaining(ReloadTimer)) / CurrentWeapon[CurrentEnumIndex].GetDefaultObject()->ReloadTime);
		OnReload.Broadcast(TimeRemainingAsPercentage);
	}
}
