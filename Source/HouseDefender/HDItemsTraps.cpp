// Copyright 2021 DME Games


#include "HDItemsTraps.h"

#include "HDEnemyMaster.h"
#include "Chaos/AABBTree.h"

AHDItemsTraps::AHDItemsTraps()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh->SetCollisionResponseToChannels(ECR_Overlap);
	ItemMesh->OnComponentBeginOverlap.AddDynamic(this, &AHDItemsTraps::OnBeginOverlap);
	ItemMesh->OnComponentEndOverlap.AddDynamic(this, &AHDItemsTraps::OnEndOverlap);
	
	ItemType = EItemType::IT_Trap;

	bIsPlaceable = true;
	HoursNeededToPlace = 1.f;
	bSlowsEnemy = false;
	bDamagesEnemy = false;
	bHasLifeAmount = true;
	StartingLife = 100.f;
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

			if (!GetWorldTimerManager().IsTimerActive(FakeTimer))
			{
				GetWorldTimerManager().SetTimer(FakeTimer, this, &AHDItemsTraps::OnTrapDestroyed, 5.0f, false, -1.f);
			}
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

			for (int32 i = OverlappingEnemies.Num() - 1; i >= 0; --i)
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
	if (!FMath::IsNearlyZero(CurrentDamageBeingDealt))
	{
		CurrentLife -= (CurrentDamageBeingDealt * DeltaTime);

		if (CurrentLife <= 0.f)
		{
			// TODO Deal with removal or destruction of mesh
		}
	}
}

void AHDItemsTraps::OnTrapDestroyed()
{
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
	//this->Destroy();
}
