// Fill out your copyright notice in the Description page of Project Settings.


#include "C_EnemyManager.h"
#include "C_Enemy.h"
#include "game/Player/C_Player.h"

#include <Kismet/GameplayStatics.h>
#include <UObject/ConstructorHelpers.h>
#include <DrawDebugHelpers.h>
#include <math.h>
#include <Components/SphereComponent.h>
#include <NavigationSystem.h>

// Sets default values
AC_EnemyManager::AC_EnemyManager()
	: m_MaxAmountOfEnemies( 10 )
	, m_pSpawnSphere( nullptr )
	, m_Enemies()
	, m_pPlayer( nullptr )
	, m_DebugSpawnRadius( false )
	, m_pTorchEnemyBP( nullptr )
	, m_pPitchforkEnemyBP( nullptr )
	, m_pTorchEnemyBP_Second(nullptr)
	, m_pPitchforkEnemyBP_Second(nullptr)
	, m_pPlayerClass( nullptr )
	, m_pWorld( nullptr )
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_pSpawnSphere = CreateDefaultSubobject<USphereComponent>( TEXT( "SpawnArea" ) );
	SetRootComponent( m_pSpawnSphere );

	// TEXT: Can be found by right-clicking blueprint and copy reference;
	static ConstructorHelpers::FObjectFinder<UClass> PlayerClass( TEXT( "Class'/Script/game.C_Player'" ) );
	if( PlayerClass.Object )
		m_pPlayerClass = PlayerClass.Object;
}

// Called when the game starts or when spawned
void AC_EnemyManager::BeginPlay()
{
	Super::BeginPlay();

	m_pWorld = GetWorld();
	m_pPlayer = Cast<AC_Player>( UGameplayStatics::GetActorOfClass( GetWorld(), m_pPlayerClass ) );
}

void AC_EnemyManager::SpawnEnemies()
{
	while( m_Enemies.Num() < m_MaxAmountOfEnemies )
	{
		FVector2D Pos2D( 0 );
		FVector3d Pos3D( 0 );
		FVector3d Temp( 0 );
		do
		{
			Pos2D = FMath::RandPointInCircle( m_pSpawnSphere->GetUnscaledSphereRadius() );
			Pos3D = FVector( Pos2D, 0 );
			Temp = UNavigationSystemV1::ProjectPointToNavigation( m_pWorld, Pos3D );
			Temp.Z = Pos3D.Z;
		} while( Temp != Pos3D );

		FTransform Transform( GetActorLocation() + Pos3D );

		bool Variation = FMath::RandBool();

		if (m_Enemies.Num() % 2 == 0)
		{
			if (Variation) 
			{
				m_Enemies.Add(m_pWorld->SpawnActor<AC_Enemy>(m_pTorchEnemyBP_Second, Transform));
			}
			else 
			{
				m_Enemies.Add(m_pWorld->SpawnActor<AC_Enemy>(m_pTorchEnemyBP, Transform));
			}
		}
		else 
		{
			if (Variation)
			{
				m_Enemies.Add(m_pWorld->SpawnActor<AC_Enemy>(m_pPitchforkEnemyBP_Second, Transform));
			}
			else {
				m_Enemies.Add( m_pWorld->SpawnActor<AC_Enemy>( m_pPitchforkEnemyBP, Transform ) );
			}
		}
	}
}

void AC_EnemyManager::DestroyEnemies()
{
	for( int i = 0; i < m_Enemies.Num(); i++ )
	{
		AC_Enemy* pEnemy = m_Enemies[i];

		if( pEnemy->GetState() != EEnemyStates::ES_PATROLING || pEnemy->GetState() == EEnemyStates::ES_IDLE )
			continue;

		pEnemy->Destroy();
		m_Enemies.RemoveAt( i );
		--i;
	}
}

// Called every frame
void AC_EnemyManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if( !m_DebugSpawnRadius )
		return;

	DrawDebugSphere( GetWorld(), GetActorLocation(), m_pSpawnSphere->GetScaledSphereRadius(), 20, FColor::Red );
}