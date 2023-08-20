// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "game/Enemy/C_Enemy.h"
#include "C_TorchEnemy.generated.h"

UCLASS()
class GAME_API AC_TorchEnemy : public AC_Enemy
{
	GENERATED_BODY()

public:
	AC_TorchEnemy();

protected:
	UFUNCTION()
		void DoAttack() override;

private:
	void DrawDebug() override;
};
