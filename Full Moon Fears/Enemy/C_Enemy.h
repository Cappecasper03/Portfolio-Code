// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include <AITypes.h>
#include <Navigation/PathFollowingComponent.h>
#include "C_Enemy.generated.h"

class UCapsuleComponent;
class UBoxComponent;
class USkeletalMeshComponent;
class AAIController;
class UFloatingPawnMovement;
class AC_Player;
class UC_AnimInstance;

UENUM()
enum class EEnemyStates // class removes intellisense error
{
	ES_IDLE,
	ES_PATROLING,
	ES_CHASING,
	ES_SEARCHING,
	ES_ATTACKING
};

UCLASS( Abstract )
class GAME_API AC_Enemy : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AC_Enemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnAttack();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void WhileBreathing();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnFootstep();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnKilledEnemy();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnCalledForBackup();

	UFUNCTION()
		void MoveTowardsTarget();
	UFUNCTION()
		void RandomizeNewPatrolPoint();
	UFUNCTION()
		void Idle( float DeltaTime );
	UFUNCTION()
		void Patroling( float DeltaTime );
	virtual void Chasing( float DeltaTime );
	UFUNCTION()
		void Searching( float DeltaTime );
	virtual void Attacking();
	virtual void DoAttack() {};
	UFUNCTION()
		void CallForBackup();
	UFUNCTION()
		bool IsAtTargetPoint();
	UFUNCTION()
		bool FailedToMove();

	UFUNCTION()
		bool IsPlayerInRange( double Range, bool DirectionMatters = true );
	virtual float GetAttackRange() { return m_AttackRange; }

	UFUNCTION()
		void StateMachine( float DeltaTime );

	UFUNCTION()
		void OnOverlapBegin( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult );

	void GetPos();
	void ReleasePos();

	UPROPERTY( EditAnyWhere, Category = "Ranges" )
		float m_VisionRange;
	UPROPERTY( EditAnyWhere, Category = "Ranges" )
		float m_HearingRange;
	UPROPERTY( EditAnyWhere, Category = "Ranges" )
		float m_CallOnRadius;

	UPROPERTY( EditAnyWhere, Category = "Ranges" )
		float m_AttackRange;
	bool m_pHasHitPlayer;

	UPROPERTY( EditAnyWhere, Category = "Speeds" )
		float m_MovementSpeed;

	UPROPERTY( EditAnyWhere, Category = "Speeds" )
		float m_RotationSpeed;

	UPROPERTY( EditAnyWhere, Category = "Times" )
		int m_MaxIdleTime;

	UPROPERTY( EditAnyWhere, Category = "Ranges" )
		float m_PatrolRadius;

	FVector3d m_TargetPoint;
	bool m_HasLastPlayerPosition;

	bool m_CanSeeHidingPlayer;

	UPROPERTY( EditDefaultsOnly )
		EEnemyStates m_CurrentState;

	UPROPERTY( EditDefaultsOnly )
		UCapsuleComponent* m_pCapsuleCollider;
	UPROPERTY( EditDefaultsOnly )
		UBoxComponent* m_pWeaponCollider;

	UPROPERTY( EditDefaultsOnly )
		UBoxComponent* m_pRightFootCollider;
	UPROPERTY( EditDefaultsOnly )
		UBoxComponent* m_pLeftFootCollider;

	UPROPERTY( EditDefaultsOnly )
		USkeletalMeshComponent* m_pMesh;

	AC_Player* m_pPlayer;

	AAIController* m_pAIController;
	UFloatingPawnMovement* m_pFloatingPawnMovement;
	FAIMoveRequest m_MoveRequest;
	FPathFollowingRequestResult m_MoveRequestResult;

	UPROPERTY( EditAnyWhere, Category = "Debug" )
		bool m_DebugVisionRange;
	UPROPERTY( EditAnyWhere, Category = "Debug" )
		bool m_DebugHearingRange;
	UPROPERTY( EditAnyWhere, Category = "Debug" )
		bool m_DebugAttackRange;
	UPROPERTY( EditAnyWhere, Category = "Debug" )
		bool m_DebugCallOnRange;
	UPROPERTY( EditAnyWhere, Category = "Debug" )
		bool m_DebugPatrolRange;

	UWorld* m_pWorld;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pIdleAnimation;
	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pRunAnimation;
	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pAttackAnimation;

	FVector3d m_PatrolCenter;

	UPROPERTY(EditAnyWhere, Category = "Locked In Pos")
		FVector m_LockedInPos;

	UPROPERTY(VisibleAnyWhere, Category = "Locked In Pos")
		bool m_HasLockedInPos;

	UC_AnimInstance* m_pAnimInstance;

private:
	UClass* m_pPlayerClass;
	UClass* m_pEnemyClass;


	FVector3d m_OldTargetPoint;

	float m_IdleTimer;
	int m_CurrentIdleTime;
	bool m_Idling;

	FVector3d m_LastPosition;

	virtual void DrawDebug() {};

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;

	UFUNCTION()
		EEnemyStates GetState() { return m_CurrentState; }

	UFUNCTION()
		void CallOn( FVector lastKnownPlayerPos );
};
