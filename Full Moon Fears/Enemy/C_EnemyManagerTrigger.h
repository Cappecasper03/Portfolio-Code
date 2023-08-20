// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_EnemyManagerTrigger.generated.h"

class AC_EnemyManager;
class AC_Player;
class USphereComponent;

UCLASS()
class GAME_API AC_EnemyManagerTrigger : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AC_EnemyManagerTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY( EditAnyWhere )
		USphereComponent* m_pTriggerSphere;

	TArray<AC_EnemyManager*> m_EnemyManagers;

	AC_Player* m_pPlayer;

	UPROPERTY( EditAnyWhere )
		bool m_DebugTriggerRadius;

private:
	UClass* m_pPlayerClass;
	UClass* m_pEnemyManagerBP;

	UWorld* m_pWorld;

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;

};
