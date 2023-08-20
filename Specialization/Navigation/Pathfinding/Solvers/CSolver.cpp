#include <tgSystem.h>

#include "CSolver.h"
#include "Navigation/CNavMesh.h"

#include <tgCProfiling.h>
#include <tgCLine3D.h>
#include <tgCLine2D.h>

CSolver::CSolver( CNavMesh* pNavMesh, tgCMutex* pMutex )
    : m_FunneledPath()
    , m_pNavMesh( pNavMesh )
    , m_pStartNode( nullptr )
    , m_pGoalNode( nullptr )
    , m_pCurrentNode( nullptr )
    , m_pMutex( pMutex )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL
}

CSolver::EResult CSolver::FindPath( SNavMeshNode* pStartNode, SNavMeshNode* pGoalNode, const tgBool& rStopping )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    m_pMutex->Lock();
    m_FunneledPath.clear();
    m_pStartNode            = pStartNode;
    m_pStartNode->IsClosed  = true;
    m_pStartNode->IsVisited = true;
    m_pCurrentNode          = m_pStartNode;
    m_pGoalNode             = pGoalNode;
    m_pMutex->Unlock();

    tgBool Searching = false;
    while( !Searching )
    {
        if( rStopping )
            break;

        Searching = Search();
    }

    std::vector<SNavMeshNode*> Path;
    const tgBool               FoundPath = GetPath( Path, rStopping );

    Clear();
    if( FoundPath && !Path.empty() )
    {
        FunnelPath( Path );
        return PATH_FOUND;
    }
    else
    {
        m_FunneledPath.clear();
        return PATH_NOT_FOUND;
    }
}

tgBool CSolver::GetPath( std::vector<SNavMeshNode*>& rPath, const tgBool& rStopping )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    rPath.clear();
    if( m_pCurrentNode != m_pGoalNode )
        return false;

    SNavMeshNode* pNode = m_pCurrentNode;

    while( pNode != m_pStartNode )
    {
        rPath.insert( rPath.begin(), pNode );

        if( pNode->pParentNode )
            pNode = pNode->pParentNode;
        else
            return false;

        if( rStopping )
        {
            rPath.clear();
            return false;
        }
    }

    rPath.insert( rPath.begin(), pNode );
    return true;
}

void CSolver::FunnelPath( const std::vector<SNavMeshNode*>& rPath )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    m_FunneledPath.clear();
    m_FunneledPath.push_back( &m_pStartNode->Center );

    const std::vector<tgCLine3D>& rNavMeshEdges = m_pNavMesh->GetEdges();

    tgUInt32 StartIndex        = 0;
    tgSInt32 NodesLeftToFunnel = rPath.size();
    while( NodesLeftToFunnel > 1 )
    {
        tgUInt32 GoalIndex         = rPath.size() - 1;
        tgUInt32 IndexChangeAmount = GoalIndex - StartIndex;
        do
        {
            if( GoalIndex >= rPath.size() )
            {
                GoalIndex = rPath.size();
                break;
            }

            tgBool        LinesIntersected = false;
            const tgCV3D* pStartPoint      = &rPath[StartIndex]->Center;
            const tgCV3D* pGoalPoint       = &rPath[GoalIndex]->Center;
            tgCV3D        StartToGoal      = *pGoalPoint - *pStartPoint;
            tgCLine2D     StartToGoalLine( tgCV2D( pStartPoint->x, pStartPoint->z ), tgCV2D( pGoalPoint->x, pGoalPoint->z ) );

            for( const tgCLine3D& rNavMeshEdge : rNavMeshEdges )
            {
                tgCLine2D Edge( tgCV2D( rNavMeshEdge.GetStart().x, rNavMeshEdge.GetStart().z ), tgCV2D( rNavMeshEdge.GetEnd().x, rNavMeshEdge.GetEnd().z ) );

                if( Edge.Intersect( StartToGoalLine ) )
                {
                    LinesIntersected = true;
                    break;
                }
            }

            if( IndexChangeAmount > 1 )
                IndexChangeAmount /= 2;

            if( LinesIntersected )
                GoalIndex -= IndexChangeAmount;
            else
            {
                if( IndexChangeAmount <= 2 )
                {
                    StartIndex = GoalIndex;
                    m_FunneledPath.push_back( pGoalPoint );
                    break;
                }

                GoalIndex += IndexChangeAmount;
            }
        } while( true );

        NodesLeftToFunnel = rPath.size() - GoalIndex;
    }

    m_FunneledPath.push_back( &m_pGoalNode->Center );
}

void CSolver::Clear( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    std::vector<SNavMeshNode>& rNodes = m_pNavMesh->GetNodes();
    for( SNavMeshNode& rNode : rNodes )
    {
        rNode.pParentNode = nullptr;
        rNode.IsClosed    = false;
        rNode.IsVisited   = false;
    }
}
