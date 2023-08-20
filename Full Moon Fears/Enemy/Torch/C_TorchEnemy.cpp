// Fill out your copyright notice in the Description page of Project Settings.


#include "C_TorchEnemy.h"
#include "game/Player/C_Player.h"
#include "game/Interactable/Tree/C_InteractableTree.h"
#include "game/Animation/C_AnimInstance.h"

#include <Components/BoxComponent.h>

AC_TorchEnemy::AC_TorchEnemy()
{
	m_AttackRange = 150;
	m_CanSeeHidingPlayer = true;

	m_pWeaponCollider = CreateDefaultSubobject<UBoxComponent>( TEXT( "WeaponCollider" ) );
	m_pWeaponCollider->SetupAttachment( m_pMesh, FName( "L_Point_01" ) );

	m_pRightFootCollider->SetupAttachment( m_pMesh, FName( "TorchRig_RightFoot" ) );

	m_pLeftFootCollider->SetupAttachment( m_pMesh, FName( "TorchRig_LeftFoot" ) );
}

void AC_TorchEnemy::DoAttack()
{
	if( !m_pAnimInstance->IsPlaying( m_pAttackAnimation ) || !m_pAnimInstance->IsPlaying() )
	{
		m_pAnimInstance->ChangeAnimation( m_pAttackAnimation, false );
		OnAttack();
		m_pHasHitPlayer = false;
	}

	if( m_pHasHitPlayer )
		return;

	m_pWeaponCollider->UpdateOverlaps();
	if( !m_pWeaponCollider->IsOverlappingActor( m_pPlayer ) )
		return;

	AC_InteractableTree* pInteractableTree = m_pPlayer->GetUsedInteractableTree();
	if( !pInteractableTree )
	{
		m_pHasHitPlayer = true;
		m_pPlayer->SetOnFire();
	}
	else
		pInteractableTree->SetOnFire( true );
}

void AC_TorchEnemy::DrawDebug()
{
	FVector Location = GetActorLocation();
	int Segments = 15;

	if( m_DebugVisionRange )
		DrawDebugSphere( m_pWorld, Location, m_VisionRange, Segments, FColor::Green );

	if( m_DebugHearingRange )
		DrawDebugSphere( m_pWorld, Location, m_HearingRange, Segments, FColor::Black );

	if( m_DebugAttackRange )
		DrawDebugSphere( m_pWorld, Location, m_AttackRange, Segments, FColor::Red );

	if( m_DebugCallOnRange )
		DrawDebugSphere( m_pWorld, Location, m_CallOnRadius, Segments, FColor::Cyan );

	if( m_DebugPatrolRange )
		DrawDebugSphere( m_pWorld, m_PatrolCenter, m_PatrolRadius, Segments, FColor::Blue );
}
