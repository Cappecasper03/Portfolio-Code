#pragma once

#include "Navigation/SNavMeshNode.h"
#include "Navigation/CNavMesh.h"

#include <tgCV3D.h>

#include <tgMemoryDisable.h>
#include <vector>
#include <algorithm>
#include <tgMemoryEnable.h>

class CSolver
{
public:
    enum EResult
    {
        PATH_FOUND
        ,PATH_NOT_FOUND
    };

    CSolver( CNavMesh* pNavMesh, tgCMutex* pMutex );
    virtual ~CSolver( void ) = default;

    EResult FindPath( SNavMeshNode* pStartNode, SNavMeshNode* pGoalNode, const tgBool& rStopping = false );

    std::vector<const tgCV3D*>& GetFunneledPath( void ) { return m_FunneledPath; }

protected:
    virtual tgBool Search( void ) = 0;

    tgBool GetPath( std::vector<SNavMeshNode*>& rPath, const tgBool& rStopping );

    void FunnelPath( const std::vector<SNavMeshNode*>& rPath );

    virtual void Clear( void );

    std::vector<const tgCV3D*> m_FunneledPath;

    CNavMesh* m_pNavMesh;

    SNavMeshNode* m_pStartNode;
    SNavMeshNode* m_pGoalNode;
    SNavMeshNode* m_pCurrentNode;

    tgCMutex* m_pMutex;
};
