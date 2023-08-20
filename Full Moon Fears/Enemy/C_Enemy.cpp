// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Enemy.h"
#include "game/Player/C_Player.h"
#include "game/Animation/C_AnimInstance.h"

#include <Components/CapsuleComponent.h>
#include <Components/BoxComponent.h>
#include <Kismet/GameplayStatics.h>
#include <UObject/ConstructorHelpers.h>
#include <Engine/SkeletalMesh.h>
#include <DrawDebugHelpers.h>
#include <AIController.h>
#include <GameFramework/FloatingPawnMovement.h>
#include <math.h>
#include <Kismet/KismetMathLibrary.h>

// Sets default values
AC_Enemy::AC_Enemy()
	: m_VisionRange( 1000 )
	, m_HearingRange( 500 )
	, m_CallOnRadius( 5000 )
	, m_AttackRange( 0 )
	, m_pHasHitPlayer( false )
	, m_MovementSpeed( 500 )
	, m_RotationSpeed( 10 )
	, m_MaxIdleTime( 5 )
	, m_PatrolRadius( 1000 )
	, m_TargetPoint( 0 )
	, m_HasLastPlayerPosition( false )
	, m_CanSeeHidingPlayer( false )
	, m_CurrentState( EEnemyStates::ES_IDLE )
	, m_pCapsuleCollider( nullptr )
	, m_pWeaponCollider( nullptr )
	, m_pRightFootCollider( nullptr )
	, m_pLeftFootCollider( nullptr )
	, m_pMesh( nullptr )
	, m_pPlayer( nullptr )
	, m_pAIController( nullptr )
	, m_pFloatingPawnMovement( nullptr )
	, m_MoveRequestResult()
	, m_DebugVisionRange( false )
	, m_DebugHearingRange( false )
	, m_DebugAttackRange( false )
	, m_DebugCallOnRange( false )
	, m_DebugPatrolRange( false )
	, m_pWorld( nullptr )
	, m_pIdleAnimation( nullptr )
	, m_pRunAnimation( nullptr )
	, m_pAttackAnimation( nullptr )
	, m_PatrolCenter( 0 )
	, m_LockedInPos( 0 )
	, m_HasLockedInPos( false )
	, m_pAnimInstance( nullptr )
	, m_pPlayerClass( nullptr )
	, m_pEnemyClass( nullptr )
	, m_OldTargetPoint( m_TargetPoint )
	, m_IdleTimer( 0 )
	, m_CurrentIdleTime( 0 )
	, m_Idling( false )
	, m_LastPosition( 0 )
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_pCapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollider" ) );
	SetRootComponent( m_pCapsuleCollider );

	m_pMesh = CreateDefaultSubobject<USkeletalMeshComponent>( TEXT( "Mesh" ) );
	m_pMesh->SetupAttachment( m_pCapsuleCollider );

	m_pRightFootCollider = CreateDefaultSubobject<UBoxComponent>( TEXT( "RightFoot" ) );
	m_pLeftFootCollider = CreateDefaultSubobject<UBoxComponent>( TEXT( "LeftFoot" ) );

	// TEXT: Can be found by right-clicking blueprint and copy reference;
	static ConstructorHelpers::FObjectFinder<UClass> PlayerClass( TEXT( "Class'/Script/game.C_Player'" ) );
	if( PlayerClass.Object )
		m_pPlayerClass = PlayerClass.Object;

	static ConstructorHelpers::FObjectFinder<UClass> EnemyClass( TEXT( "Class'/Script/game.C_Enemy'" ) );
	if( EnemyClass.Object )
		m_pEnemyClass = EnemyClass.Object;

	m_pFloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>( TEXT( "FloatingPawnMovement" ) );

	m_MoveRequest.SetGoalLocation( m_TargetPoint );
	m_MoveRequest.SetAllowPartialPath( true );
	m_MoveRequest.SetUsePathfinding( true );
	m_MoveRequest.SetProjectGoalLocation( true );
}

// Called when the game starts or when spawned
void AC_Enemy::BeginPlay()
{
	Super::BeginPlay();

	m_pWorld = GetWorld();
	m_pPlayer = Cast<AC_Player>( UGameplayStatics::GetActorOfClass( m_pWorld, m_pPlayerClass ) );

	m_pMesh->SetRelativeRotation( FRotator( 0, -90, 0 ) );

	m_pFloatingPawnMovement->MaxSpeed = m_MovementSpeed;

	m_pAIController = Cast<AAIController>( GetController() );

	FHitResult HitResult;
	FVector3d Offset( 0, 0, 1000 );
	FCollisionQueryParams Params;
	Params.AddIgnoredActor( this );
	m_pWorld->LineTraceSingleByChannel( HitResult, GetActorLocation() + Offset, GetActorLocation() - Offset, ECC_WorldStatic, Params );
	SetActorLocation( HitResult.Location - m_pMesh->GetRelativeLocation() ); // Relative location is negative so this will actually do addition
	m_PatrolCenter = GetActorLocation();

	RandomizeNewPatrolPoint();
	m_MoveRequest.SetAcceptanceRadius( m_AttackRange - 80 );

	m_pAnimInstance = Cast<UC_AnimInstance>( m_pMesh->GetAnimInstance() );
	m_pAnimInstance->ChangeAnimation( m_pIdleAnimation );

	m_pRightFootCollider->OnComponentBeginOverlap.AddDynamic( this, &AC_Enemy::OnOverlapBegin );
	m_pLeftFootCollider->OnComponentBeginOverlap.AddDynamic( this, &AC_Enemy::OnOverlapBegin );
}

void AC_Enemy::MoveTowardsTarget()
{
	m_TargetPoint.Z = GetActorLocation().Z;
	if( m_TargetPoint != m_OldTargetPoint )
	{
		m_MoveRequest.UpdateGoalLocation( m_TargetPoint );
		m_OldTargetPoint = m_TargetPoint;
	}

	if( m_pAIController )
		m_MoveRequestResult = m_pAIController->MoveTo( m_MoveRequest );

	if( FailedToMove() )
	{
		RandomizeNewPatrolPoint();
		return;
	}

	if( IsAtTargetPoint() )
		return;

	if( !m_pAnimInstance->IsPlaying( m_pRunAnimation ) || !m_pAnimInstance->IsPlaying() )
		m_pAnimInstance->ChangeAnimation( m_pRunAnimation );

	if( GetActorLocation() == m_LastPosition )
		return;

	float DeltaTime = m_pWorld->GetDeltaSeconds();

	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation( m_LastPosition, GetActorLocation() );
	Rotator = FMath::RInterpTo( GetActorRotation(), Rotator, DeltaTime, m_RotationSpeed );
	SetActorRotation( Rotator );
	m_LastPosition = GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor( this );
	m_pWorld->LineTraceSingleByChannel( HitResult, GetActorLocation() + FVector( 0, 0, 50 ), GetActorLocation() - FVector( 0, 0, 1000 ), ECC_Visibility, Params );
	if( !HitResult.GetActor() && !HitResult.GetActor()->ActorHasTag( "Walkable" ) )
		return;

	float Height = m_pCapsuleCollider->GetScaledCapsuleHalfHeight();
	FVector Location = FMath::VInterpTo( GetActorLocation(), HitResult.ImpactPoint + FVector( 0, 0, Height ), DeltaTime, 10 );
	SetActorLocation( Location );
}

void AC_Enemy::RandomizeNewPatrolPoint()
{
	FVector2D Pos2D = FMath::RandPointInCircle( m_PatrolRadius );
	FVector3d Pos3D( Pos2D, GetActorLocation().Z );
	m_TargetPoint = m_PatrolCenter + Pos3D;
	m_MoveRequestResult.Code = EPathFollowingRequestResult::RequestSuccessful;
}

void AC_Enemy::Idle( float DeltaTime )
{
	m_HasLastPlayerPosition = false;

	if( !m_pAnimInstance->IsPlaying( m_pIdleAnimation ) || !m_pAnimInstance->IsPlaying() )
		m_pAnimInstance->ChangeAnimation( m_pIdleAnimation );

	if( ( IsPlayerInRange( m_VisionRange ) || IsPlayerInRange( m_HearingRange, false ) ) && ( !m_pPlayer->IsPlayerHiding() || m_CanSeeHidingPlayer ) )
	{
		m_CurrentState = EEnemyStates::ES_CHASING;
		m_IdleTimer = 0;
		m_Idling = false;
		CallForBackup();
		return;
	}

	if( !m_Idling )
	{
		m_CurrentIdleTime = FMath::RandRange( 1, m_MaxIdleTime );
		m_Idling = true;
	}

	m_IdleTimer += DeltaTime;

	if( m_IdleTimer >= m_CurrentIdleTime )
	{
		RandomizeNewPatrolPoint();
		m_CurrentState = EEnemyStates::ES_PATROLING;
		m_IdleTimer = 0;
		m_Idling = false;
	}
}

void AC_Enemy::Patroling( float DeltaTime )
{
	m_HasLastPlayerPosition = false;

	if( ( IsPlayerInRange( m_VisionRange ) || IsPlayerInRange( m_HearingRange, false ) ) && ( !m_pPlayer->IsPlayerHiding() || m_CanSeeHidingPlayer ) )
	{
		m_CurrentState = EEnemyStates::ES_CHASING;
		CallForBackup();
		return;
	}

	if( IsAtTargetPoint() )
	{
		m_CurrentState = EEnemyStates::ES_IDLE;
		RandomizeNewPatrolPoint();
		return;
	}

	MoveTowardsTarget();
}

void AC_Enemy::Chasing( float DeltaTime )
{
	m_HasLastPlayerPosition = true;
	m_TargetPoint = m_pPlayer->GetActorLocation();

	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation( GetActorLocation(), m_TargetPoint );
	Rotator = FMath::RInterpTo( GetActorRotation(), Rotator, m_pWorld->GetDeltaSeconds(), m_RotationSpeed );
	SetActorRotation( Rotator );

	if( IsPlayerInRange( m_VisionRange ) )
	{
		if( IsPlayerInRange( GetAttackRange() ) )
		{
			m_CurrentState = EEnemyStates::ES_ATTACKING;
		}
	}
	else
	{
		m_CurrentState = EEnemyStates::ES_SEARCHING;
	}

	MoveTowardsTarget();

	if( m_pPlayer->IsDead() )
		OnKilledEnemy();
}

void AC_Enemy::Searching( float DeltaTime )
{
	if( ( IsPlayerInRange( m_VisionRange ) || IsPlayerInRange( m_HearingRange, false ) ) && ( !m_pPlayer->IsPlayerHiding() || m_CanSeeHidingPlayer ) )
	{
		CallForBackup();
		m_CurrentState = EEnemyStates::ES_CHASING;
	}
	else if( IsAtTargetPoint() )
	{
		m_CurrentState = EEnemyStates::ES_IDLE;
	}

	MoveTowardsTarget();

	if( m_pPlayer->IsDead() )
		OnKilledEnemy();
}

void AC_Enemy::Attacking()
{
	m_HasLastPlayerPosition = true;
	m_pAIController->StopMovement();

	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation( GetActorLocation(), m_TargetPoint );
	Rotator = FMath::RInterpTo( GetActorRotation(), Rotator, m_pWorld->GetDeltaSeconds(), m_RotationSpeed );
	SetActorRotation( Rotator );

	bool IsAttacking = false;
	if( m_pAnimInstance->IsPlaying() && m_pAnimInstance->IsPlaying( m_pAttackAnimation ) )
		IsAttacking = true;

	DoAttack();

	if( IsPlayerInRange( GetAttackRange() ) )
	{
		m_CurrentState = EEnemyStates::ES_ATTACKING;
	}
	else if( IsPlayerInRange( m_VisionRange ) || IsPlayerInRange( m_HearingRange, false ) )
	{
		if( !IsAttacking )
		{
			m_CurrentState = EEnemyStates::ES_CHASING;
		}
	}
	else
	{
		if( !IsAttacking )
		{
			m_CurrentState = EEnemyStates::ES_IDLE;
		}
	}

	if( m_pPlayer->IsDead() )
		OnKilledEnemy();
}

void AC_Enemy::CallForBackup()
{
	if( m_HasLastPlayerPosition )
		return;

	OnCalledForBackup();
	TArray<AActor*> EnemyArray;
	UGameplayStatics::GetAllActorsOfClass( m_pWorld, m_pEnemyClass, EnemyArray );

	FVector PlayerPos = m_pPlayer->GetActorLocation();

	for( AActor* pEnemy : EnemyArray )
	{
		FVector PlayerToEnemy = pEnemy->GetActorLocation() - PlayerPos;
		double Distance = PlayerToEnemy.Length();

		if( Distance < m_CallOnRadius )
			Cast<AC_Enemy>( pEnemy )->CallOn( m_TargetPoint );
	}
}

bool AC_Enemy::IsAtTargetPoint()
{
	if( m_MoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal )
		return true;

	return false;
}

bool AC_Enemy::FailedToMove()
{
	if( m_MoveRequestResult == EPathFollowingRequestResult::Failed )
		return true;

	return false;
}

void AC_Enemy::CallOn( FVector lastKnownPlayerPos )
{
	m_TargetPoint = lastKnownPlayerPos;

	m_CurrentState = EEnemyStates::ES_CHASING;
	m_HasLastPlayerPosition = true;
}

bool AC_Enemy::IsPlayerInRange( double Range, bool DirectionMatters )
{
	FVector PlayerPos = m_pPlayer->GetActorLocation();

	FVector EnemyForwardDir = GetActorForwardVector();

	FVector EnemyToPlayer = PlayerPos - GetActorLocation();
	double Distance = EnemyToPlayer.Length();

	double DotProduct = EnemyToPlayer.DotProduct( EnemyForwardDir, EnemyToPlayer );

	if( Distance <= Range && ( DotProduct > 0 || !DirectionMatters ) )
		return true;

	return false;
}

void AC_Enemy::StateMachine( float DeltaTime )
{
	switch( m_CurrentState )
	{
		case EEnemyStates::ES_IDLE:
			Idle( DeltaTime );
			break;

		case EEnemyStates::ES_PATROLING:
			Patroling( DeltaTime );
			break;

		case EEnemyStates::ES_CHASING:
			Chasing( DeltaTime );
			m_pPlayer->SetIsChased();
			break;

		case EEnemyStates::ES_SEARCHING:
			Searching( DeltaTime );
			break;

		case EEnemyStates::ES_ATTACKING:
			Attacking();
			m_pPlayer->SetIsChased();
			break;
	}
}

void AC_Enemy::OnOverlapBegin( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult )
{
	if( OtherActor == this && OtherActor->ActorHasTag( "Walkable" ) )
		return;

	OnFootstep();
}

void AC_Enemy::GetPos()
{
	if( !m_HasLockedInPos )
	{
		m_LockedInPos = m_pPlayer->GetUnoccupiedPos();
		m_HasLockedInPos = true;
	}
}

void AC_Enemy::ReleasePos()
{
	if( m_HasLockedInPos )
	{
		m_pPlayer->LetGoOfPos( m_LockedInPos );
		m_LockedInPos = FVector( 0, 0, 0 );
		m_HasLockedInPos = false;
	}
}

// Called every frame
void AC_Enemy::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	StateMachine( DeltaTime );
	WhileBreathing();

	FRotator Rotator = GetActorRotation();
	Rotator.Pitch = 0;
	SetActorRotation( Rotator );

	DrawDebug();
}