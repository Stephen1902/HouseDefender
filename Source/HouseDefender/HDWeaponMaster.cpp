// Copyright 2021 DME Games


#include "HDWeaponMaster.h"

// Sets default values
AHDWeaponMaster::AHDWeaponMaster()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Component"));
	SetRootComponent(MeshComponent);

	WeaponName = FText::FromString("Default Name");
	DamagePerShot = 20.f;
	VulnerableBonus = 1.5f;
	ArmourPenalty = 1.0f;
	FireRate = 0.2f;
	FireDistance = 500.f;
	MagazineSize = 20;
	ReloadTime = 0.5f;
	WeaponType = EWeaponType::CW_Pistol;
	bPlayerHasThisWeapon = false;
}

// Called when the game starts or when spawned
void AHDWeaponMaster::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmoInClip = MagazineSize;
	TotalAmmo = MagazineSize;
}

// Called every frame
void AHDWeaponMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

