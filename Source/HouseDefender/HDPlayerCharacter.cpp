// Copyright 2021 DME Games


#include "HDPlayerCharacter.h"

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

// Called when the game starts or when spawned
void AHDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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
}

void AHDPlayerCharacter::Look(float AxisValue)
{
//	if (AxisValue != 0.f)
	{
		FVector WorldLoc;
		FVector WorldRot;

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->DeprojectMousePositionToWorld(WorldLoc, WorldRot);
			CurrentRotation = WorldRot.Z;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No controller found on this player"));
		}
		
	}
}

