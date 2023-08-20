// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyManagerTrigger.h"
#include "game/Player/C_Player.h"
#include "C_EnemyManager.h"

#include <Kismet/GameplayStatics.h>
#include <UObject/ConstructorHelpers.h>
#include <DrawDebugHelpers.h>
#include <math.h>
#include <Components/SphereComponent.h>

// Sets default values
AC_EnemyManagerTrigger::AC_EnemyManagerTrigger()
	: m_pTriggerSphere( nullptr )
	, m_EnemyManagers()
	, m_pPlayer( nullptr )
	, m_DebugTriggerRadius( false )
	, m_pPlayerClass( nullptr )
	, m_pEnemyManagerBP( nullptr )
	, m_pWorld( nullptr )
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_pTriggerSphere = CreateDefaultSubobject<USphereComponent>( TEXT( "TriggerArea" ) );
	SetRootComponent( m_pTriggerSphere );

	// TEXT: Can be found by right-clicking blueprint and copy reference;
	static ConstructorHelpers::FObjectFinder<UClass> PlayerBP( TEXT( "Class'/Script/game.C_Player'" ) );
	if( PlayerBP.Object )
		m_pPlayerClass = PlayerBP.Object;

	static ConstructorHelpers::FObjectFinder<UClass> EnemyManagerBP( TEXT( "Class'/Script/game.C_EnemyManager'" ) );
	if( EnemyManagerBP.Object )
		m_pEnemyManagerBP = EnemyManagerBP.Object;
}

// Called when the game starts or when spawned
void AC_EnemyManagerTrigger::BeginPlay()
{
	Super::BeginPlay();

	m_pWorld = GetWorld();

	m_pPlayer = Cast<AC_Player>( UGameplayStatics::GetActorOfClass( m_pWorld, m_pPlayerClass ) );

	TArray<AActor*> EnemyManagers;
	UGameplayStatics::GetAllActorsOfClass( m_pWorld, m_pEnemyManagerBP, EnemyManagers );

	for( AActor* pEnemyManager : EnemyManagers )
	{
		float DistanceToManager = FVector3d::Distance( pEnemyManager->GetActorLocation(), GetActorLocation() );

		if( DistanceToManager < m_pTriggerSphere->GetUnscaledSphereRadius() )
		{
			m_EnemyManagers.Add( Cast<AC_EnemyManager>( pEnemyManager ) );
		}
	}
}

// Called every frame
void AC_EnemyManagerTrigger::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	float DistanceToPlayer = FVector3d::Distance( m_pPlayer->GetActorLocation(), GetActorLocation() );
	float Radius = m_pTriggerSphere->GetUnscaledSphereRadius();

	if( DistanceToPlayer < Radius )
	{
		for( AC_EnemyManager* pEnemyManager : m_EnemyManagers )
			pEnemyManager->SpawnEnemies();
	}
	else if( DistanceToPlayer > Radius )
	{
		for( AC_EnemyManager* pEnemyManager : m_EnemyManagers )
			pEnemyManager->DestroyEnemies();
	}

	if( !m_DebugTriggerRadius )
		return;

	DrawDebugSphere( GetWorld(), GetActorLocation(), m_pTriggerSphere->GetUnscaledSphereRadius(), 20, FColor::Red );
}

