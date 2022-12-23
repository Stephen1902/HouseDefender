// Copyright 2021 DME Games


#include "HDItemsTraps.h"
#include "HDEnemyMaster.h"
#include "Components/WidgetComponent.h"

AHDItemsTraps::AHDItemsTraps()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh->SetCollisionResponseToChannels(ECR_Overlap);
	ItemMesh->OnComponentBeginOverlap.AddDynamic(this, &AHDItemsTraps::OnBeginOverlap);
	ItemMesh->OnComponentEndOverlap.AddDynamic(this, &AHDItemsTraps::OnEndOverlap);
	
	ItemType = EItemType::IT_Trap;
/** Keep only in case of having a visible health widget 
	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	WidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComp->SetDrawSize(FVector2D(100.f, 20.f));
	WidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
*/
	bIsPlaceable = true;
	HoursNeededToPlace = 1.f;
	bSlowsEnemy = false;
	bDamagesEnemy = false;
	bHasLifeAmount = true;
	StartingLife = 100.f;
	HoursNeededToRepair = 0.25f;
//	ColourOfHealthBar = FLinearColor(0.112136f, 0.128103f, 1.f, 3.0f);
	bHasBeenDestroyed = false;
}

void AHDItemsTraps::BeginPlay()
{
	Super::BeginPlay();

	CurrentLife = StartingLife;
	SetWidgetInfo();


}

void AHDItemsTraps::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForDamage(DeltaTime);
}

void AHDItemsTraps::OnBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AHDEnemyMaster* OverlappedEnemy = Cast<AHDEnemyMaster>(OtherActor))
	{
		if (!OverlappedEnemy->GetIsDead())
		{
			OverlappingEnemies.Add(OverlappedEnemy);
		
			if (bSlowsEnemy)
			{
				OverlappedEnemy->SetSpeedReductionFromTraps(EnemySlowRate);
				if (FMath::IsNearlyZero(EnemySlowRate))
				{
					OverlappedEnemy->SetIsBlocked(true);
				}
			}

			if (bDamagesEnemy)
			{
				OverlappedEnemy->SetDamageTaken(EnemyDamageRate);
			}

			// If the enemy can damage this trap, add damage to total damage
			if (!FMath::IsNearlyZero(OverlappedEnemy->GetDamageDealtToTraps()))
			{
				CurrentDamageBeingDealt += OverlappedEnemy->GetDamageDealtToTraps();
			}

/**			if (!GetWorldTimerManager().IsTimerActive(FakeTimer))
			{
				GetWorldTimerManager().SetTimer(FakeTimer, this, &AHDItemsTraps::OnTrapDestroyed, 5.0f, false, -1.f);
			}
*/
		}
	}
}

void AHDItemsTraps::OnEndOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex)
{
	if (AHDEnemyMaster* OverlappedEnemy = Cast<AHDEnemyMaster>(OtherActor))
	{
		if (OverlappingEnemies.Num() > 0)
		{
			int32 MatchFound = -1;

			for (int32 i = 0; i < OverlappingEnemies.Num(); ++i)
			{
				if (OverlappingEnemies[i] == OverlappedEnemy)
				{
					MatchFound = i;
				}
			}

			if (MatchFound > -1)
			{
				OverlappingEnemies.RemoveAt(MatchFound);
			}
	
		}
		
		if (bSlowsEnemy)
		{
			OverlappedEnemy->SetSpeedReductionFromTraps(1.f);
			if (FMath::IsNearlyZero(EnemySlowRate))
			{
				OverlappedEnemy->SetIsBlocked(false);
			}
		}

		if (bDamagesEnemy)
		{
			OverlappedEnemy->SetDamageTaken(0.f);
		}

		// If the enemy that has stopped overlapping was damaging the trap, reduce the damage
		if (!FMath::IsNearlyZero(OverlappedEnemy->GetDamageDealtToTraps()))
		{
			CurrentDamageBeingDealt -= OverlappedEnemy->GetDamageDealtToTraps();
		}
	}
}

void AHDItemsTraps::CheckForDamage(float DeltaTime)
{
	//GEngine->AddOnScreenDebugMessage(0, 0.f, FColor::Green, (TEXT("%s"), *FString::SanitizeFloat(OverlappingEnemies.Num())));
	if (!FMath::IsNearlyZero(CurrentDamageBeingDealt))
	{
		CurrentLife -= (CurrentDamageBeingDealt * DeltaTime);
		
		if (CurrentLife <= 0.f && !bHasBeenDestroyed)
		{
			OnTrapDestroyed();
		}
/** Keep only in case of having a visible health widget 
		else if (WidgetComp)
		{
			if (!WidgetComp->IsVisible())
			{
				WidgetComp->SetVisibility(true);
			}
			const float CurrentLifeAsPercent = 1 - ((StartingLife - CurrentLife) / StartingLife);
			OnTrapHit.Broadcast(UserWidget, CurrentLifeAsPercent);
		}
*/
	}
}

void AHDItemsTraps::SetWidgetInfo()
{
/** Keep only in case of having a visible health widget 
	if (WidgetComp)
	{
		WidgetLocationAtStart = WidgetComp->GetRelativeLocation();
		UserWidget = WidgetComp->GetUserWidgetObject();
		const FBox BoundingBox = ItemMesh->GetStaticMesh()->GetBoundingBox();
		const float MeshHeight = BoundingBox.GetExtent().Z;
		WidgetComp->SetRelativeLocation(FVector(0.f, 0.f, MeshHeight + 120.f));
		WidgetComp->SetVisibility(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WidgetInfo has not been set for %s"), *GetName());
	}
*/
}

void AHDItemsTraps::OnTrapDestroyed()
{
	bHasBeenDestroyed = true;
	
	if (OverlappingEnemies.Num() > 0)
	{
		for (int32 i = OverlappingEnemies.Num() - 1; i >= 0; --i)
		{
			if (bSlowsEnemy)
			{
				OverlappingEnemies[i]->SetSpeedReductionFromTraps(1.f);
				OverlappingEnemies[i]->SetIsBlocked(false);
			}

			if (bDamagesEnemy)
			{
				OverlappingEnemies[i]->SetDamageTaken(0.f);
			}
		}
	}

	OverlappingEnemies.Empty();
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ItemMesh->SetGenerateOverlapEvents(false);

/** Keep only in case of having a visible health widget 	
	WidgetComp->SetVisibility(false);
*/
	this->Destroy();
}
