// Copyright 2021 DME Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HDEnemyMaster.generated.h"

UCLASS()
class HOUSEDEFENDER_API AHDEnemyMaster : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHDEnemyMaster();

	UPROPERTY(VisibleAnywhere, Category = "Enemy Pawn")
	class UCapsuleComponent* CapsuleComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Enemy Pawn")
	class USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Enemy Pawn")
	class UPawnMovementComponent* MovementComponent;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
