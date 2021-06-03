// Copyright 2021 DME Games


#include "HDPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "HDEnemyMaster.h"
#include "HDGameStateBase.h"
#include "HDInventoryComponent.h"
#include "HDWeaponMaster.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AHDPlayerCharacter::AHDPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));	
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Day End Spring Arm"));
	SpringArmComponent->SetupAttachment(GetMesh());
	SpringArmComponent->TargetArmLength = 100.f;
	SpringArmComponent->SetRelativeLocation(FVector(20.f, 0.f, 154.f));
	SpringArmComponent->SetRelativeRotation(FRotator(-20.f, 90.f, 0.f));
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Day End Camera"));
	CameraComp->SetupAttachment(SpringArmComponent);

	GamePlaySpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Gameplay Spring Arm"));
	GamePlaySpringArmComponent->SetupAttachment(GetMesh());
	GamePlaySpringArmComponent->SetRelativeLocation(FVector(0.f, 36.f, 164.f));
	GamePlaySpringArmComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	
	GamePlayCameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("GamePlayCamera"));
	GamePlayCameraComp->SetupAttachment(GamePlaySpringArmComponent);

	PlayerInventory = CreateDefaultSubobject<UHDInventoryComponent>(TEXT("Inventory Component"));
	
	ReloadWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	ReloadWidgetComp->SetupAttachment(GetMesh());
	ReloadWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	ReloadWidgetComp->SetDrawSize(FVector2D(64.f, 64.f));
	ReloadWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	ReloadWidgetComp->SetHiddenInGame(true);

	EndOfDayWeaponIndex = -1;

	bPlayerMoveToStart = false;
	bPlayerMoveToEnd = false;
}

int32 AHDPlayerCharacter::GetCurrentWeaponIndex() const
{
	return WeaponInfoArray[CurrentWeaponIndex].WeaponType;
}

void AHDPlayerCharacter::TryToFire()
{
	// Check if a valid weapon is available and that the day has started
	if (WeaponList.Num() > 0 &&  UKismetMathLibrary::InRange_IntInt(CurrentWeaponIndex, 0, WeaponList.Num() - 1) && GSBase->GetGameStatus() == EGameStatus::GS_DayStarted)
	{
		// Check enough time has passed since last firing against the weapon's fire rate
		if (TimeLastFired < GetWorld()->GetTimeSeconds() - WeaponInfoArray[CurrentWeaponIndex].FireRate)
		{
			// Check the weapon has ammo and they are not reloading
			if (WeaponInfoArray[CurrentWeaponIndex].CurrentAmmoInClip > 0 && !GetWorld()->GetTimerManager().IsTimerActive(ReloadTimer))
			{
				// Call the fire function
				Fire();
			}
			else
			{
				// Check there is ammo available to reload into the gun
				if (WeaponInfoArray[CurrentWeaponIndex].TotalAmmo > 0)
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

	WeaponInfoArray[CurrentWeaponIndex].CurrentAmmoInClip--;
	UpdateAmmo();
	
	FHitResult HitResult;
	//const FVector StartLoc = GetMesh()->GetSocketLocation(FName("FiringSocket"));
	const FVector StartLoc = WeaponToSpawn->MeshComponent->GetSocketLocation(FName("FiringSocket"));
	FVector EndLoc = (WeaponToSpawn->MeshComponent->GetSocketRotation(FName("FiringSocket")).Vector() * - WeaponInfoArray[CurrentWeaponIndex].FireDistance) + StartLoc;
	
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
				DamageMultiplier = WeaponInfoArray[CurrentWeaponIndex].VulnerableBonus;
				break;
			case SURFACE_FleshArmoured:
				ArmourPenalty = WeaponInfoArray[CurrentWeaponIndex].ArmourPenalty;
				break;
			}

			EnemyMaster->SetCurrentLife(WeaponInfoArray[CurrentWeaponIndex].DamagePerShot * DamageMultiplier * ArmourPenalty);			
		}
	}
}

// Called when the game starts or when spawned
void AHDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	AddWeaponInfo();
	
	PC = Cast<APlayerController>(GetController());
	GSBase = Cast<AHDGameStateBase>(GetWorld()->GetGameState());

	if (GSBase)
	{
		GSBase->OnStatusChanged.AddDynamic(this, &AHDPlayerCharacter::GameStateChanged);
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

	CheckForReloadStatus();

	if (GSBase->GetGameStatus() == EGameStatus::GS_DayStarted)
	{
		CheckForLivingEnemies();
	}

	if (bPlayerMoveToStart)
	{
		MovePlayerToDayStartPosition(DeltaTime);
	}

	if (bPlayerMoveToEnd)
	{
		MovePlayerToDayEndPosition(DeltaTime);
	}
}

// Called to bind functionality to input
void AHDPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("LookUp", this, &AHDPlayerCharacter::LookUp);
//	PlayerInputComponent->BindAxis("LookRight", this, &AHDPlayerCharacter::LookRight);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHDPlayerCharacter::TryToFire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AHDPlayerCharacter::ManualReload);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot1", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 0);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot2", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 1);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot3", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 2);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot4", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 3);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot5", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 4);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot6", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 5);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot7", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 6);
	PlayerInputComponent->BindAction<FKeyboardWeaponSelect>("SelectSlot8", IE_Pressed, this, &AHDPlayerCharacter::WeaponSelected, 7);
}

void AHDPlayerCharacter::LookUp(float AxisValue)
{
	// Safety check to ensure we have a valid controller to use
	if (!PC) { return; }

		if (AxisValue != 0.f)
	{
		// Gets the current world location of the firing socket 
		FVector2D WeaponSocketLocation;
		PC->ProjectWorldLocationToScreen(GetMesh()->GetSocketLocation(FName("FiringSocket")),WeaponSocketLocation);
		// Check mouse position on screen
		PC->GetMousePosition(MouseX, MouseY);
		// Set current rotation for animation
		CurrentRotation = FMath::Clamp(1 - (MouseY / WeaponSocketLocation.Y), -0.5f, 0.5f);

		//AddControllerPitchInput(-AxisValue);
/**		FHitResult Hit;
		PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
		
		if (Hit.IsValidBlockingHit())
		{
			const FVector HitLocation = Hit.Location;
			const FVector CurrentActorLocation = GetActorLocation();
			const FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(CurrentActorLocation, HitLocation);
			//SetActorRotation(FRotator(0.f, 0.f, NewRotation.Roll));
			PC->SetControlRotation(FRotator(0.f, 0.f, NewRotation.Roll));
		}*/
	}
}

void AHDPlayerCharacter::LookRight(float AxisValue)
{
	if (!PC) { return; }

	if (AxisValue != 0.f)
	{
		// Gets the current world location of the firing socket 
		FVector2D WeaponSocketLocation;
		PC->ProjectWorldLocationToScreen(GetMesh()->GetSocketLocation(FName("FiringSocket")),WeaponSocketLocation);
		// Check mouse position on screen
		PC->GetMousePosition(MouseX, MouseY);
		// Set current rotation for animation
	// TODO Sort animation into 3D world space
	//	CurrentRotation = FMath::Clamp(1 - (MouseY / WeaponSocketLocation.Y), -0.5f, 0.5f);

		AddControllerYawInput(AxisValue);
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

		if (TP->GetName() == TEXT("TPDropEndLocation"))
		{
			TPDropLocation = ThisTP;
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
	// This will give a "side on" view to give the player a 2D perspective of incoming enemies
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
			bPlayerMoveToStart = false;
			SetDayStartCameraLocation();
			
			GSBase->SetGameStatus(EGameStatus::GS_DayStarted);
			//GamePlayCameraComp->SetActive(true);
			//CameraComp->SetActive(false);

			if (EndOfDayWeaponIndex > -1)
			{
				WeaponSelected(EndOfDayWeaponIndex);
			}
			else
			{
				WeaponSelected(0);
			}
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
			bPlayerMoveToEnd = false;
			PC->bShowMouseCursor = true;
			GetMesh()->SetRelativeRotation(FRotator(0.f, -270.f, 0.f));
			GSBase->SetGameStatus(EGameStatus::GS_Idle);

			if (WeaponToSpawn)
			{
				WeaponToSpawn->Destroy();
			}
		}
	}
}

void AHDPlayerCharacter::CheckForLivingEnemies()
{
	if (GSBase->GameStatus == EGameStatus::GS_DayStarted && !GetWorld()->GetTimerManager().IsTimerActive(EndOfDayDelay))
	{
		TArray<AActor*> FoundEnemies;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHDEnemyMaster::StaticClass(), FoundEnemies);

		if (FoundEnemies.Num() == 0)
		{
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
	if (CurrentWeaponIndex != WeaponSelectedIn && WeaponList[WeaponSelectedIn] && WeaponInfoArray[WeaponSelectedIn].bPlayerHasThisWeapon)
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
		WeaponToSpawn = GetWorld()->SpawnActor<AHDWeaponMaster>(WeaponList[WeaponSelectedIn], GetActorLocation(), GetActorRotation(), SpawnParameters);
		const FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);

		FName SocketToAttachTo = FName("WeaponSocket");
		if (WeaponList[WeaponSelectedIn].GetDefaultObject()->WeaponType == EWeaponType::CW_Pistol)
		{
			SocketToAttachTo = FName("PistolSocket");	
		}
		WeaponToSpawn->AttachToComponent(GetMesh(), TransformRules, SocketToAttachTo);

		const FString EnumAsString = UEnum::GetValueAsString(WeaponList[CurrentWeaponIndex].GetDefaultObject()->WeaponType);
		//const int32 WeaponToSwitchOn = WeaponInfoArray[WeaponSelectedIn].WeaponType;
//		UE_LOG(LogTemp, Warning, TEXT("New Weapon selected is %s"), *EnumAsString);

		CurrentWeaponIndex = WeaponSelectedIn;

		UpdateAmmo();
	}
}

void AHDPlayerCharacter::ManualReload()
{
	if (WeaponInfoArray[CurrentWeaponIndex].TotalAmmo > 0)
	{
		// Add bullets in clip to available bullets
		WeaponInfoArray[CurrentWeaponIndex].TotalAmmo += WeaponInfoArray[CurrentWeaponIndex].CurrentAmmoInClip;
	
		// Empty the bullets in the clip
		WeaponInfoArray[CurrentWeaponIndex].CurrentAmmoInClip = 0;

		// Reload the weapon
		BeginReload();
	}
}

void AHDPlayerCharacter::BeginReload()
{
	// Prevent spamming of the reload key
	if (!GetWorld()->GetTimerManager().IsTimerActive(ReloadTimer))
	{
		const float ReloadTimerTime = WeaponInfoArray[CurrentWeaponIndex].ReloadTime;
		
		GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &AHDPlayerCharacter::Reload, ReloadTimerTime, false);
	}
}

void AHDPlayerCharacter::Reload()
{
	const int32 AmmoToAmend = FMath::Min(WeaponInfoArray[CurrentWeaponIndex].MagazineSize, WeaponInfoArray[CurrentWeaponIndex].TotalAmmo);
	WeaponInfoArray[CurrentWeaponIndex].CurrentAmmoInClip += AmmoToAmend;
	WeaponInfoArray[CurrentWeaponIndex].TotalAmmo -= AmmoToAmend;

	UpdateAmmo();
	ReloadWidgetComp->SetHiddenInGame(true);
}

void AHDPlayerCharacter::UpdateAmmo()
{
	FText NewAmmoInClip = FText::FromString(FString::FromInt(0));
	FText NewAmmoAvailable = FText::FromString(FString::FromInt(0));
	
	if (UKismetMathLibrary::InRange_IntInt(CurrentWeaponIndex, 0, WeaponList.Num() - 1))
	{
		NewAmmoInClip = FText::FromString(FString::FromInt(WeaponInfoArray[CurrentWeaponIndex].CurrentAmmoInClip));
		NewAmmoAvailable = FText::FromString(FString::FromInt(WeaponInfoArray[CurrentWeaponIndex].TotalAmmo));
	}

	OnAmmoUpdated.Broadcast(NewAmmoInClip, NewAmmoAvailable);
}

void AHDPlayerCharacter::GameStateChanged()
{
	if (GSBase)
	{
		if (GSBase->GetGameStatus() == EGameStatus::GS_DayStarting)
		{
			bPlayerMoveToStart = true;
		}

		if (GSBase->GetGameStatus() == EGameStatus::GS_DayEnding)
		{
			// Wait while dropped items to move then move the player
			GetWorld()->GetTimerManager().SetTimer(EndOfDayDelay, this, &AHDPlayerCharacter::WaitForDayEnding, 2.0f, false);
		}
	}

}

void AHDPlayerCharacter::CheckForReloadStatus()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ReloadTimer))
	{
		if (!ReloadWidgetComp->IsVisible())
		{
			ReloadWidgetComp->SetHiddenInGame(false);
		}

		const float TimeRemainingAsPercentage = ((WeaponInfoArray[CurrentWeaponIndex].ReloadTime - GetWorld()->GetTimerManager().GetTimerRemaining(ReloadTimer)) / WeaponInfoArray[CurrentWeaponIndex].ReloadTime);
		OnReload.Broadcast(TimeRemainingAsPercentage);
	}
}

void AHDPlayerCharacter::AddWeaponInfo()
{
	// Safety check to ensure array is empty
	WeaponInfoArray.Empty();
	WeaponInfoArray.SetNum(WeaponList.Num());
		
	for (int32 i = 0; i < WeaponList.Num(); ++i)
	{
		// Check that the current weapon slot isn't set to "None"
		if (WeaponList[i] != nullptr)
		{
			CurrentWeaponIndex = i;
			WeaponInfoArray[i].WeaponName =	WeaponList[CurrentWeaponIndex].GetDefaultObject()->WeaponName;
			WeaponInfoArray[i].DamagePerShot = WeaponList[CurrentWeaponIndex].GetDefaultObject()->DamagePerShot;
			WeaponInfoArray[i].VulnerableBonus = WeaponList[CurrentWeaponIndex].GetDefaultObject()->VulnerableBonus;
			WeaponInfoArray[i].ArmourPenalty = WeaponList[CurrentWeaponIndex].GetDefaultObject()->ArmourPenalty;
			WeaponInfoArray[i].FireRate = WeaponList[CurrentWeaponIndex].GetDefaultObject()->FireRate;
			WeaponInfoArray[i].FireDistance = WeaponList[CurrentWeaponIndex].GetDefaultObject()->FireDistance;
			WeaponInfoArray[i].MagazineSize = WeaponList[CurrentWeaponIndex].GetDefaultObject()->MagazineSize;
			WeaponInfoArray[i].TotalAmmo = WeaponList[CurrentWeaponIndex].GetDefaultObject()->TotalAmmo;
			WeaponInfoArray[i].CurrentAmmoInClip = WeaponList[CurrentWeaponIndex].GetDefaultObject()->CurrentAmmoInClip;
			WeaponInfoArray[i].ReloadTime = WeaponList[CurrentWeaponIndex].GetDefaultObject()->ReloadTime;
			WeaponInfoArray[i].WeaponType = static_cast<int>(WeaponList[CurrentWeaponIndex].GetDefaultObject()->WeaponType);
			WeaponInfoArray[i].bPlayerHasThisWeapon = WeaponList[CurrentWeaponIndex].GetDefaultObject()->bPlayerHasThisWeapon;

			const FString CurrentWeaponAsText = UEnum::GetValueAsString(WeaponList[CurrentWeaponIndex].GetDefaultObject()->WeaponType);
			//UE_LOG(LogTemp, Warning, TEXT("Weapon Type at slot %i is %s"), WeaponInfoArray[i].WeaponType, *CurrentWeaponAsText);
		}
	}
}

void AHDPlayerCharacter::WaitForDayEnding()
{
	// Set Camera to behind the player's shoulder
	MoveCameraToNewLocation(GetWorld()->GetFirstPlayerController()->GetPawn());
	GetWorld()->GetTimerManager().ClearTimer(EndOfDayDelay);
	
	bPlayerMoveToEnd = true;
}
