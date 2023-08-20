//////////////////////////////////////////////////////////////////////////////////////////
//  File name: CLevel.h                                                                 //
//  Created:   2022-02-18 15:31:40                                                      //
//                                                                                      //
//                                                                                      //
//  Copyright (c) 2022 Tension Graphics AB                                              //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CLEVEL_H__
#define __CLEVEL_H__

#include <tgCSingleton.h>

class CPathfindingManager;
class COctree;
class CPlayer;
class CNavMesh;
class CEnemyManager;

class CLevel : public tgCSingleton<CLevel>
{
public:

	// Constructor / Destructor
	 CLevel( void );
	~CLevel( void );

//////////////////////////////////////////////////////////////////////////

	void Update( const tgFloat DeltaTime );

	void Render( void );

//////////////////////////////////////////////////////////////////////////

	tgCWorld* GetCollisionWorld( void ) const { return m_pCollisionWorld; }
	tgCWorld* GetNavigationWorld( void ) const { return m_pNavigationWorld; }

	CNavMesh* GetNavMesh( void ) const { return m_pNavMesh; }
	COctree*  GetOctree( void ) const { return m_pOctree; }

	CPlayer* GetPlayer( void ) const { return m_pPlayer; }

	CPathfindingManager* GetPathfindingManager( void ) const { return m_pPathfindingManager; }

	void ToggleDebugRender( void ) { DoDebugRender = !DoDebugRender; }

private:
	tgCWorld* m_pCollisionWorld;
	tgCWorld* m_pNavigationWorld;

	CNavMesh* m_pNavMesh;
	COctree*  m_pOctree;

	CPlayer* m_pPlayer;

	CPathfindingManager* m_pPathfindingManager;
	CEnemyManager*       m_pEnemyManager;

	tgBool DoDebugRender;

};	// CLevel

#endif // __CLEVEL_H__
