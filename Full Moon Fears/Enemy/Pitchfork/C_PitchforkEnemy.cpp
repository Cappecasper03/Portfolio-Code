// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PitchforkEnemy.h"
#include "game/Player/C_Player.h"
#include "game/Animation/C_AnimInstance.h"

#include <UObject/ConstructorHelpers.h>
#include <math.h>
#include <Engine/StaticMesh.h>
#include <Components/BoxComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include <AIController.h>

AC_PitchforkEnemy::AC_PitchforkEnemy()
	: m_ThrowRange( 1000 )
	, m_ShouldThrow( false )
	, m_PitchforkIsOnGround( false )
	, m_pPitchforkMesh( nullptr )
	, m_DebugThrowRange( false )
	, m_pThrowAnimation( nullptr )
	, m_pPickupAnimation( nullptr )
	, m_PitchforkGravity( 30 )
	, m_PitchforkSpeed( 500 )
	, m_PitchforkRotationSpeed( 500 )
	, m_InHandTransform()
	, m_PickupPitchforkTimer( 0 )
	, m_PickupMaxTime( 10 )
{
	m_AttackRange = 250;

	m_pPitchforkMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "PitchforkMesh" ) );
	m_pPitchforkMesh->SetupAttachment( m_pMesh, FName( "R_Index_01" ) );

	m_pWeaponCollider = CreateDefaultSubobject<UBoxComponent>( TEXT( "WeaponCollider" ) );
	m_pWeaponCollider->SetupAttachment( m_pPitchforkMesh );

	m_pRightFootCollider->SetupAttachment( m_pMesh, FName( "PitchforkRig_RightFoot" ) );

	m_pLeftFootCollider->SetupAttachment( m_pMesh, FName( "PitchforkRig_LeftFoot" ) );
}

void AC_PitchforkEnemy::BeginPlay()
{
	Super::BeginPlay();

	if( m_ThrowRange > m_VisionRange )
		m_ThrowRange = m_VisionRange;

	m_ShouldThrow = FMath::RandBool();
	if( m_ShouldThrow )
		m_MoveRequest.SetAcceptanceRadius( m_ThrowRange - 80 );
	else
		m_MoveRequest.SetAcceptanceRadius( m_AttackRange - 80 );

	m_InHandTransform = m_pPitchforkMesh->GetRelativeTransform();
}

void AC_PitchforkEnemy::DoAttack()
{
	if( ( !m_pAnimInstance->IsPlaying( m_pThrowAnimation ) && !m_pAnimInstance->IsPlaying( m_pAttackAnimation ) ) || !m_pAnimInstance->IsPlaying() )
	{
		if( m_ShouldThrow )
		{
			m_pAnimInstance->ChangeAnimation( m_pThrowAnimation, false );
			OnThrow();
		}
		else
		{
			m_pAnimInstance->ChangeAnimation( m_pAttackAnimation, false );
			OnAttack();
		}

		m_pHasHitPlayer = false;
		m_ShouldThrow = FMath::RandBool();

		if( m_ShouldThrow )
			m_MoveRequest.SetAcceptanceRadius( m_ThrowRange - 80 );
		else
			m_MoveRequest.SetAcceptanceRadius( m_AttackRange - 80 );
	}

	if( m_pAnimInstance->GetAnimTime( m_pThrowAnimation ) >= .46f &&
		m_pAnimInstance->IsPlaying( m_pThrowAnimation ) )
	{
		FDetachmentTransformRules DetachmentRules( EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false );
		m_pPitchforkMesh->DetachFromComponent( DetachmentRules );
		m_CurrentState = EEnemyStates::ES_CHASING;
		m_PitchforkIsOnGround = false;
	}

	if( !m_pHasHitPlayer )
		CheckForHitOnPlayer();
}

void AC_PitchforkEnemy::UpdatePitchfork()
{
	m_PickupPitchforkTimer += m_pWorld->GetDeltaSeconds();

	FHitResult HitResult;
	FVector Location = m_pPitchforkMesh->GetRelativeLocation();
	m_pWorld->LineTraceSingleByChannel( HitResult, Location, Location + m_pPitchforkMesh->GetRightVector() * 50, ECC_Visibility );
	if( HitResult.bBlockingHit )
	{
		m_PitchforkIsOnGround = true;
		return;
	}

	float DeltaTime = m_pWorld->GetDeltaSeconds();

	m_pPitchforkMesh->AddLocalRotation( FRotator( 0, 0, m_PitchforkRotationSpeed * DeltaTime ) );
	m_pPitchforkMesh->AddRelativeRotation( FRotator( -m_PitchforkGravity * DeltaTime, 0, 0 ) );
	m_pPitchforkMesh->AddRelativeLocation( m_pPitchforkMesh->GetForwardVector() * m_PitchforkSpeed * DeltaTime );

	if( !m_pHasHitPlayer )
		CheckForHitOnPlayer();
}

void AC_PitchforkEnemy::CheckForHitOnPlayer()
{
	m_pWeaponCollider->UpdateOverlaps();

	if( !m_pWeaponCollider->IsOverlappingActor( m_pPlayer ) )
		return;

	m_pHasHitPlayer = true;

	m_pPlayer->TakeDamage();
}

void AC_PitchforkEnemy::Chasing( float DeltaTime )
{
	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation( GetActorLocation(), m_pPlayer->GetActorLocation() );
	Rotator = FMath::RInterpTo( GetActorRotation(), Rotator, m_pWorld->GetDeltaSeconds(), m_RotationSpeed );
	SetActorRotation( Rotator );

	if( !m_pPitchforkMesh->IsAttachedTo( m_pMesh ) )
	{
		m_MoveRequest.SetAcceptanceRadius( 0 );
		m_TargetPoint = m_pPitchforkMesh->GetComponentTransform().GetLocation();
		UpdatePitchfork();

		bool TookToLong = false;
		if( m_PickupPitchforkTimer > m_PickupMaxTime )
			TookToLong = true;

		if( ( m_PitchforkIsOnGround && IsAtTargetPoint() ) || TookToLong )
		{
			if( !m_pAnimInstance->IsPlaying( m_pPickupAnimation ) || !m_pAnimInstance->IsPlaying() )
				m_pAnimInstance->ChangeAnimation( m_pPickupAnimation, false );

			if( ( m_pAnimInstance->GetAnimTime( m_pPickupAnimation ) >= .56f && m_pAnimInstance->IsPlaying( m_pPickupAnimation ) ) || TookToLong )
			{
				FAttachmentTransformRules AttachmentRules( EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false );
				m_pPitchforkMesh->AttachToComponent( m_pMesh, AttachmentRules, FName( "R_Index_01" ) );
				m_pPitchforkMesh->SetRelativeTransform( m_InHandTransform );
				m_PickupPitchforkTimer = 0;
			}
		}
	}
	else
	{
		if( m_pAnimInstance->IsPlaying( m_pPickupAnimation ) && m_pAnimInstance->IsPlaying() )
			return;

		m_HasLastPlayerPosition = true;
		m_TargetPoint = m_pPlayer->GetActorLocation();

		if( IsPlayerInRange( m_VisionRange ) )
		{
			if( IsPlayerInRange( GetAttackRange() ) )
			{
				m_CurrentState = EEnemyStates::ES_ATTACKING;
			}

			if( m_ShouldThrow )
				m_MoveRequest.SetAcceptanceRadius( m_ThrowRange - 80 );
			else
				m_MoveRequest.SetAcceptanceRadius( m_AttackRange - 80 );
		}
		else
		{
			m_MoveRequest.SetAcceptanceRadius( 0 );
			m_CurrentState = EEnemyStates::ES_SEARCHING;
		}
	}

	if( m_pPitchforkMesh->IsAttachedTo( m_pMesh ) || m_PitchforkIsOnGround )
		MoveTowardsTarget();

	if( m_pPlayer->IsDead() )
		OnKilledEnemy();
}

void AC_PitchforkEnemy::Attacking()
{
	m_HasLastPlayerPosition = true;
	m_pAIController->StopMovement();

	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation( GetActorLocation(), m_pPlayer->GetActorLocation() );
	Rotator = FMath::RInterpTo( GetActorRotation(), Rotator, m_pWorld->GetDeltaSeconds(), m_RotationSpeed );
	Rotator.Yaw -= 1;
	SetActorRotation( Rotator );

	bool IsAttacking = false;
	if( m_pAnimInstance->IsPlaying() && ( m_pAnimInstance->IsPlaying( m_pAttackAnimation ) ||
										  m_pAnimInstance->IsPlaying( m_pThrowAnimation ) ||
										  m_pAnimInstance->IsPlaying( m_pPickupAnimation ) ) )
	{
		IsAttacking = true;
	}

	DoAttack();

	if( !m_pPitchforkMesh->IsAttachedTo( m_pMesh ) )
		return;

	if( IsPlayerInRange( GetAttackRange() ) )
	{
		m_CurrentState = EEnemyStates::ES_ATTACKING;
	}
	else if( IsPlayerInRange( m_VisionRange ) || IsPlayerInRange( m_HearingRange, false ) )
	{
		if( !IsAttacking )
			m_CurrentState = EEnemyStates::ES_CHASING;
	}
	else
	{
		if( !IsAttacking )
			m_CurrentState = EEnemyStates::ES_IDLE;
	}

	if( m_pPlayer->IsDead() )
		OnKilledEnemy();
}

float AC_PitchforkEnemy::GetAttackRange()
{
	if( m_ShouldThrow )
		return m_ThrowRange;

	return m_AttackRange;
}

void AC_PitchforkEnemy::DrawDebug()
{
	FVector Location = GetActorLocation();
	int Segments = 15;

	if( m_DebugVisionRange )
		DrawDebugSphere( m_pWorld, Location, m_VisionRange, Segments, FColor::Green );

	if( m_DebugHearingRange )
		DrawDebugSphere( m_pWorld, Location, m_HearingRange, Segments, FColor::Black );

	if( m_DebugAttackRange )
		DrawDebugSphere( m_pWorld, Location, m_AttackRange, Segments, FColor::Red );

	if( m_DebugThrowRange )
		DrawDebugSphere( m_pWorld, Location, m_ThrowRange, Segments, FColor::Magenta );

	if( m_DebugCallOnRange )
		DrawDebugSphere( m_pWorld, Location, m_CallOnRadius, Segments, FColor::Cyan );

	if( m_DebugPatrolRange )
		DrawDebugSphere( m_pWorld, m_PatrolCenter, m_PatrolRadius, Segments, FColor::Blue );
}
