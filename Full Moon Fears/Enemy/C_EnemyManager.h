// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_EnemyManager.generated.h"

class AC_Enemy;
class AC_Player;
class USphereComponent;

UCLASS()
class GAME_API AC_EnemyManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AC_EnemyManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY( EditAnyWhere )
		int m_MaxAmountOfEnemies;
	UPROPERTY( EditAnyWhere )
		USphereComponent* m_pSpawnSphere;

	TArray<AC_Enemy*> m_Enemies;

	AC_Player* m_pPlayer;

	UPROPERTY( EditAnyWhere )
		bool m_DebugSpawnRadius;

private:
	UPROPERTY( EditDefaultsOnly )
		UClass* m_pTorchEnemyBP;
	UPROPERTY( EditDefaultsOnly )
		UClass* m_pPitchforkEnemyBP;
	UPROPERTY(EditDefaultsOnly)
		UClass* m_pTorchEnemyBP_Second;
	UPROPERTY(EditDefaultsOnly)
		UClass* m_pPitchforkEnemyBP_Second;

	UClass* m_pPlayerClass;

	UWorld* m_pWorld;

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;

	UFUNCTION()
		void SpawnEnemies();

	UFUNCTION()
		void DestroyEnemies();

};
