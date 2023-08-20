//////////////////////////////////////////////////////////////////////////////////////////
//  File name: CLevel.cpp                                                               //
//  Created:   2022-02-18 15:31:49                                                      //
//                                                                                      //
//                                                                                      //
//  Copyright (c) 2022 Tension Graphics AB                                              //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

#include <tgSystem.h>

#include "CLevel.h"
#include "CPlayer.h"
#include "Navigation/CNavMesh.h"
#include "Octree/COctree.h"
#include "Managers/CWorldManager.h"
#include "Enemy/CEnemyManager.h"
#include "Navigation/Pathfinding/CPathfindingManager.h"

#include <tgCTextureManager.h>
#include <tgCProfiling.h>
#include <tgCDebugManager.h>

////////////////////////////// CLevel //////////////////////////////
//                                                                //
//  Info:
//                                                                //
//*/////////////////////////////////////////////////////////////////
CLevel::CLevel( void )
	: m_pCollisionWorld( nullptr )
	, m_pNavigationWorld( nullptr )
	, m_pNavMesh( nullptr )
	, m_pOctree( nullptr )
	, m_pPlayer( nullptr )
	, m_pPathfindingManager( nullptr )
	, m_pEnemyManager( nullptr )
	, DoDebugRender( false )
{
#if !defined( FINAL )
	tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL
	
	// Load worlds
	CWorldManager& rWorldManager = CWorldManager::GetInstance();
	m_pCollisionWorld            = rWorldManager.LoadWorld( "worlds/city_collision.tfw", "Collision" );
	m_pNavigationWorld           = rWorldManager.LoadWorld( "worlds/city_navigation.tfw", "Navigation" );

	rWorldManager.SetActiveWorld( m_pCollisionWorld );

	m_pNavMesh = new CNavMesh( "Navigation" );
	m_pOctree  = new COctree( 6 );

	m_pPlayer = new CPlayer;

	m_pPathfindingManager = new CPathfindingManager();
	m_pEnemyManager       = new CEnemyManager();
} // */ // CLevel


////////////////////////////// ~CLevel //////////////////////////////
//                                                                 //
//  Info:
//                                                                 //
//*//////////////////////////////////////////////////////////////////
CLevel::~CLevel( void )
{
#if !defined( FINAL )
	tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL
	
	delete m_pEnemyManager;
	delete m_pPathfindingManager;
	delete m_pPlayer;
	delete m_pOctree;
	delete m_pNavMesh;

	CWorldManager& rWorldManager = CWorldManager::GetInstance();
	if( m_pNavigationWorld )
		rWorldManager.DestroyWorld( m_pNavigationWorld );
	
	if( m_pCollisionWorld )
		rWorldManager.DestroyWorld( m_pCollisionWorld );

}	// */ // ~CLevel


////////////////////////////// Update //////////////////////////////
//                                                                //
//  Info:
//                                                                //
//*/////////////////////////////////////////////////////////////////
void
CLevel::Update( const tgFloat DeltaTime )
{
#if !defined( FINAL )
	tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL
	
	if( m_pPlayer )
		m_pPlayer->Update( DeltaTime );

	if( m_pPathfindingManager )
		m_pPathfindingManager->Update( DeltaTime );

	if( m_pEnemyManager )
		m_pEnemyManager->Update( DeltaTime );
	
} // */ // Update

void CLevel::Render( void )
{
#if !defined( FINAL )
	tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

	tgCDebugManager& rDebugManager = tgCDebugManager::GetInstance();

	rDebugManager.AddText2D( tgCV2D( 10, 50 ), tgCColor::Yellow, "1: Toggle Camera/Player" );
	rDebugManager.AddText2D( tgCColor::Yellow, "2: Toggle Debug/Statistics" );

	if( m_pEnemyManager )
		m_pEnemyManager->Render( DoDebugRender );

	if( m_pPlayer )
		m_pPlayer->Render();

	if( !DoDebugRender )
		return;

	rDebugManager.AddText2D( tgCV2D( 10, 80 ), tgCColor::Yellow, "Statistics:" );

	if( m_pEnemyManager )
	{
		rDebugManager.AddText2D( tgCColor::Yellow, tgCString( "Enemies:               %d", m_pEnemyManager->GetNumEnemies() ) );
		rDebugManager.AddText2D( tgCColor::Yellow, tgCString( "Rendered Enemies:      %d", m_pEnemyManager->GetNumRenderedEnemies() ) );
		rDebugManager.AddText2D( tgCColor::Yellow, "" );
	}

	if( m_pPathfindingManager )
	{
		rDebugManager.AddText2D( tgCColor::Yellow, "Pathfinding Times:" );

		m_pPathfindingManager->GetMutex().Lock();
		const tgDouble LatestPathfindingTime = m_pPathfindingManager->GetLatestPathfindingTime();
		const tgSize   AverageNum            = m_pPathfindingManager->GetPathfindingTimes().size();
		const tgDouble AverageTime           = m_pPathfindingManager->GetAveragePathfindingTime();
		m_pPathfindingManager->GetMutex().Unlock();

		rDebugManager.AddText2D( tgCColor::Yellow, tgCString( "Latest Path:           %1.3f ms", LatestPathfindingTime ) );
		rDebugManager.AddText2D( tgCColor::Yellow, tgCString( "Average of %d:        %1.3f ms", AverageNum, AverageTime ) );
		rDebugManager.AddText2D( tgCColor::Yellow, "" );

		rDebugManager.AddText2D( tgCColor::Yellow, tgCString( "Existing Paths:        %d", m_pPathfindingManager->GetPaths().size() + 1 ) );
		rDebugManager.AddText2D( tgCColor::Yellow, tgCString( "Used Paths:            %d", m_pPathfindingManager->GetAmountOfUsedPaths() ) );
		rDebugManager.AddText2D( tgCColor::Yellow, "" );
	}

	if( m_pOctree )
	{
		rDebugManager.AddText2D( tgCColor::Yellow, tgCString( "Existing Octree Nodes: %d", m_pOctree->GetDeepestNodes().size() ) );
		rDebugManager.AddText2D( tgCColor::Yellow, tgCString( "Used Octree Nodes:     %d", m_pOctree->GetNodesWithObjects().size() ) );
		rDebugManager.AddText2D( tgCColor::Yellow, "" );
	}

	if( m_pPathfindingManager )
		m_pPathfindingManager->Render();

	if( m_pNavMesh )
		m_pNavMesh->Render();

	if( m_pOctree )
		m_pOctree->Render();
}