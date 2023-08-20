#pragma once

#include "../CNavMesh.h"

#include <tgCMutex.h>

#include <tgMemoryDisable.h>
#include <memory>
#include <tgMemoryEnable.h>

class tgCThread;
class CSolver;
class COctree;

struct SOctreeNode;

class CPathfindingManager
{
public:
    struct SPathInfo
    {
        tgBool                                      PathfindingCompleted;
        std::shared_ptr<std::vector<const tgCV3D*>> SharedPath;
        std::weak_ptr<std::vector<const tgCV3D*>>   WeakPath;

        tgCV3D StartPosition;
        tgCV3D GoalPosition;

        SNavMeshNode* pNavMeshStartNode;
        SNavMeshNode* pNavMeshGoalNode;

        const SOctreeNode* pOctreeStartNode;
    };

    CPathfindingManager( void );
    ~CPathfindingManager( void );

    void Update( const tgFloat DeltaTime );

    void Render( void );

    std::vector<SPathInfo>& GetPaths( void ) { return m_Paths; }
    tgUInt32                GetAmountOfUsedPaths( void );
    SPathInfo&              GetCurrentPath( void ) { return m_CurrentPath; }

    tgCMutex& GetMutex( void ) { return m_Mutex; }

    const tgDouble&              GetLatestPathfindingTime( void ) { return m_LatestPathfindingTime; }
    tgDouble                     GetAveragePathfindingTime( void );
    const std::vector<tgDouble>& GetPathfindingTimes( void ) { return m_PathfindingTimes; }

private:
    struct SThreadParams
    {
        CPathfindingManager* pPathfindingManager;

        tgBool IsWorking;
        tgBool IsStarting;
        tgBool IsStopping;
    };

    static void FindPathThread( tgCThread* pThread );

    void UpdatePaths( void );

    std::vector<SPathInfo> m_Paths;

    CSolver* m_pSolver;

    SPathInfo m_CurrentPath;

    SThreadParams m_ThreadParams;

    tgCThread* m_pPathfinderThread;
    tgCMutex   m_Mutex;

    tgFloat m_RestartTimer;

    tgDouble              m_LatestPathfindingTime;
    std::vector<tgDouble> m_PathfindingTimes;
};
