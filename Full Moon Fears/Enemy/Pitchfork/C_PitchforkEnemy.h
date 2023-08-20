// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "game/Enemy/C_Enemy.h"
#include "C_PitchforkEnemy.generated.h"

class UStaticMeshComponent;

UCLASS()
class GAME_API AC_PitchforkEnemy : public AC_Enemy
{
	GENERATED_BODY()

public:
	AC_PitchforkEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnThrow();

	UFUNCTION()
		void DoAttack() override;
	UFUNCTION()
		void UpdatePitchfork();
	UFUNCTION()
		void CheckForHitOnPlayer();

	UFUNCTION()
		void Chasing( float DeltaTime ) override;
	UFUNCTION()
		void Attacking() override;

	UFUNCTION()
		float GetAttackRange() override;

	UPROPERTY( VisibleAnywhere, Category = "Ranges" )
		float m_ThrowRange;
	bool m_ShouldThrow;
	bool m_PitchforkIsOnGround;

	UPROPERTY( EditDefaultsOnly )
		UStaticMeshComponent* m_pPitchforkMesh;

	UPROPERTY( EditAnyWhere, Category = "Debug" )
		bool m_DebugThrowRange;

private:
	void DrawDebug() override;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pThrowAnimation;
	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pPickupAnimation;

	UPROPERTY( EditAnyWhere, Category = "Pitchfrok" )
		float m_PitchforkGravity;
	UPROPERTY( EditAnyWhere, Category = "Pitchfrok" )
		float m_PitchforkSpeed;
	UPROPERTY( EditAnyWhere, Category = "Pitchfrok" )
		float m_PitchforkRotationSpeed;

	FTransform m_InHandTransform;

	float m_PickupPitchforkTimer;
	UPROPERTY( EditAnyWhere, Category = "Pitchfork" )
		float m_PickupMaxTime;
};
