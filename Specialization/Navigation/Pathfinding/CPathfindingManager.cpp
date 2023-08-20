#include <tgSystem.h>

#include "CPathfindingManager.h"
#include "Solvers/CAStarSolver.h"
#include "Octree/COctree.h"
#include "Octree/IOctreeObject.h"
#include "Specialization/CLevel.h"

#include <tgCProfiling.h>
#include <tgCDebugManager.h>
#include <tgCLine3D.h>
#include <tgCThread.h>
#include <tgCTimer.h>

CPathfindingManager::CPathfindingManager( void )
    : m_Paths()
    , m_pSolver( nullptr )
    , m_CurrentPath{}
    , m_ThreadParams()
    , m_pPathfinderThread( nullptr )
    , m_Mutex( "PathfindingSystem" )
    , m_RestartTimer( 0 )
    , m_LatestPathfindingTime( 0 )
    , m_PathfindingTimes()
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    m_pSolver = new CAStarSolver( CLevel::GetInstance().GetNavMesh(), &m_Mutex );

    UpdatePaths();

    m_ThreadParams.pPathfindingManager = this;
    m_ThreadParams.IsWorking           = true;
    m_ThreadParams.IsStarting          = false;
    m_ThreadParams.IsStopping          = false;

    m_pPathfinderThread = new tgCThread( "PathSolver", FindPathThread, tgCThread::PRIORITY_HIGHEST, 65536U, &m_ThreadParams );
}

CPathfindingManager::~CPathfindingManager( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    m_ThreadParams.IsStopping = true;
    m_ThreadParams.IsWorking  = false;

    delete m_pPathfinderThread;

    if( m_pSolver )
    {
        const CSolver* pTemp = m_pSolver;
        m_pSolver            = nullptr;
        delete pTemp;
    }
}

void CPathfindingManager::Update( const tgFloat DeltaTime )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    UpdatePaths();

    m_RestartTimer += DeltaTime;

    tgCMutexScopeLock ScopeLock( m_Mutex );
    if( m_CurrentPath.PathfindingCompleted || m_RestartTimer > .05f )
    {
        if( m_CurrentPath.PathfindingCompleted )
        {
            m_CurrentPath.PathfindingCompleted = false;
            m_Paths.push_back( std::move( m_CurrentPath ) );
        }

        m_RestartTimer = 0;

        if( m_Paths.empty() )
            return;

        m_CurrentPath = std::move( m_Paths.front() );

        while( !m_CurrentPath.WeakPath.expired() || m_CurrentPath.WeakPath.lock() )
        {
            m_Paths.erase( m_Paths.begin() );
            m_Paths.push_back( std::move( m_CurrentPath ) );
            m_CurrentPath = std::move( m_Paths.front() );
        }

        m_Paths.erase( m_Paths.begin() );

        const std::vector<IOctreeObject*>& rOctreeObjects = m_CurrentPath.pOctreeStartNode->Objects;
        if( rOctreeObjects.empty() )
            return;

        m_CurrentPath.StartPosition = *rOctreeObjects[tgMathRandom( 0, static_cast<tgSInt32>( rOctreeObjects.size() - 1 ) )]->GetPosition();

        CNavMesh* pNavMesh              = CLevel::GetInstance().GetNavMesh();
        m_CurrentPath.pNavMeshGoalNode  = pNavMesh->GetNode( m_CurrentPath.GoalPosition );
        m_CurrentPath.pNavMeshStartNode = pNavMesh->GetNode( m_CurrentPath.StartPosition );

        m_ThreadParams.IsStarting = true;
    }
}

void CPathfindingManager::Render( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgCDebugManager& rDebugManager = tgCDebugManager::GetInstance();
    const tgCV3D     Offset( 0, .1f, 0 );
    tgCLine3D        Line( 0 );

    for( const SPathInfo& rPathInfo : m_Paths )
    {
        if( !rPathInfo.SharedPath || rPathInfo.SharedPath == rPathInfo.WeakPath.lock() || rPathInfo.WeakPath.lock() || rPathInfo.SharedPath->size() < 2 )
            continue;

        for( tgSize i = 0; i < rPathInfo.SharedPath->size() - 1; ++i )
        {
            Line.Set( *rPathInfo.SharedPath->at( i ) + Offset, *rPathInfo.SharedPath->at( i + 1 ) + Offset );
            rDebugManager.AddLine3D( Line, tgCColor::White );
        }
    }
}

tgUInt32 CPathfindingManager::GetAmountOfUsedPaths( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgUInt32 AmountOfUsedPaths = 0;

    for( SPathInfo& rPathInfo : m_Paths )
    {
        if( !rPathInfo.WeakPath.lock() )
            AmountOfUsedPaths++;
    }

    return AmountOfUsedPaths;
}

tgDouble CPathfindingManager::GetAveragePathfindingTime( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgCMutexScopeLock ScopeMutex( m_Mutex );
    if( m_PathfindingTimes.empty() )
        return 0;

    tgDouble AverageTime = 0;
    for( const tgDouble& rTime : m_PathfindingTimes )
        AverageTime += rTime;

    AverageTime /= static_cast<tgDouble>( m_PathfindingTimes.size() );
    return AverageTime;
}

void CPathfindingManager::FindPathThread( tgCThread* pThread )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    SThreadParams* pParams = static_cast<SThreadParams*>( pThread->GetUserData() );

    CPathfindingManager* pPathfindingManager = pParams->pPathfindingManager;
    SPathInfo&           rPathInfo           = pPathfindingManager->m_CurrentPath;
    tgCMutex&            rMutex              = pPathfindingManager->m_Mutex;

    while( pParams->IsWorking )
    {
        if( !pParams->IsStarting )
        {
            tgSleep( 10 );
            continue;
        }

#if !defined( FINAL )
        tgProfilingScope( __TG_FUNC__ "::Pathfinding" );
#endif // !FINAL


        tgCTimer      Timer;
        SNavMeshNode* pStartNode = nullptr;
        SNavMeshNode* pGoalNode  = nullptr;
        {
            tgCMutexScopeLock ScopeMutex( rMutex );
            pParams->IsStopping = false;
            pParams->IsStarting = false;
            pStartNode          = rPathInfo.pNavMeshStartNode;
            pGoalNode           = rPathInfo.pNavMeshGoalNode;
            if( !pStartNode || !pGoalNode || ( pStartNode == pGoalNode ) )
            {
                rPathInfo.PathfindingCompleted = true;
                continue;
            }
        }

        const CSolver::EResult Result = pPathfindingManager->m_pSolver->FindPath( pStartNode, pGoalNode, pParams->IsStopping );

        tgCMutexScopeLock ScopeMutex( rMutex );
        if( Result == CSolver::PATH_FOUND )
            rPathInfo.SharedPath.reset( new std::vector<const tgCV3D*>( pPathfindingManager->m_pSolver->GetFunneledPath() ) );

        pPathfindingManager->m_LatestPathfindingTime = Timer.GetLifeTime() * 1000;
        pPathfindingManager->m_PathfindingTimes.push_back( pPathfindingManager->m_LatestPathfindingTime );

        if( pPathfindingManager->m_PathfindingTimes.size() > 100 )
            pPathfindingManager->m_PathfindingTimes.erase( pPathfindingManager->m_PathfindingTimes.begin() );

        rPathInfo.PathfindingCompleted = true;
    }
}

void CPathfindingManager::UpdatePaths( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    {
        tgCMutexScopeLock                  ScopeMutex( m_Mutex );
        const std::vector<IOctreeObject*>& rOctreeObjects = m_CurrentPath.pOctreeStartNode->Objects;
        if( m_CurrentPath.pOctreeStartNode && !rOctreeObjects.empty() )
            m_CurrentPath.StartPosition = *rOctreeObjects[tgMathRandom( 0, static_cast<tgSInt32>( rOctreeObjects.size() - 1 ) )]->GetPosition();
    }

    for( tgSize i = 0; i < m_Paths.size(); ++i )
    {
        SPathInfo&                         rPathInfo      = m_Paths[i];
        const std::vector<IOctreeObject*>& rOctreeObjects = rPathInfo.pOctreeStartNode->Objects;

        if( rOctreeObjects.empty() )
        {
            m_Paths.erase( m_Paths.begin() + i );

            if( i != 0 )
                i--;
        }
        else if( rPathInfo.WeakPath.expired() )
            rPathInfo.WeakPath.reset();
    }

    const std::vector<SOctreeNode*> Nodes = CLevel::GetInstance().GetOctree()->GetNodesWithObjects();
    for( const SOctreeNode* pOctreeNode : Nodes )
    {
        {
            tgCMutexScopeLock ScopeMutex( m_Mutex );
            if( pOctreeNode->Objects.empty() || m_CurrentPath.pOctreeStartNode == pOctreeNode )
                continue;
        }

        tgBool IsPathExisting = false;
        for( const SPathInfo& rPathInfo : m_Paths )
        {
            if( pOctreeNode == rPathInfo.pOctreeStartNode )
            {
                IsPathExisting = true;
                break;
            }
        }

        if( !IsPathExisting )
        {
            SPathInfo PathInfo{};
            PathInfo.PathfindingCompleted = false;
            PathInfo.StartPosition        = *pOctreeNode->Objects[0]->GetPosition();
            PathInfo.GoalPosition         = tgCV3D::Zero;
            PathInfo.pNavMeshStartNode    = nullptr;
            PathInfo.pNavMeshGoalNode     = nullptr;
            PathInfo.pOctreeStartNode     = pOctreeNode;

            m_Paths.push_back( std::move( PathInfo ) );
        }
    }

    for( SPathInfo& rPathInfo1 : m_Paths )
    {
        if( !rPathInfo1.SharedPath || rPathInfo1.SharedPath->size() < 2 || !rPathInfo1.WeakPath.expired() )
            continue;

        for( tgUInt32 i = 0; i < rPathInfo1.SharedPath->size() - 1; ++i )
        {
            tgCLine3D Line( *rPathInfo1.SharedPath->at( i ), *rPathInfo1.SharedPath->at( i + 1 ) );

            for( SPathInfo& rPathInfo2 : m_Paths )
            {
                if( rPathInfo1.SharedPath == rPathInfo2.SharedPath )
                    continue;

                if( rPathInfo1.WeakPath.expired() && Line.Intersect( rPathInfo2.pOctreeStartNode->Box ) )
                    rPathInfo2.WeakPath = rPathInfo1.SharedPath;
            }
        }
    }
}
