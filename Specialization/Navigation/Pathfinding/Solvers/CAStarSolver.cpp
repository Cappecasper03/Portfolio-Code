#include <tgSystem.h>

#include "CAStarSolver.h"

#include <tgCProfiling.h>

#include <tgMemoryDisable.h>
#include <algorithm>
#include <tgMemoryEnable.h>

tgBool SortAscendingF( const SAStarNode* pNode1, const SAStarNode* pNode2 ) { return pNode1->F < pNode2->F; }

CAStarSolver::CAStarSolver( CNavMesh* pNavMesh, tgCMutex* pMutex )
    : CSolver( pNavMesh, pMutex )
    , m_SortedByF()
    , m_AStarNodes()
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgCMutexScopeLock ScopeMutex( *pMutex );

    for( tgUInt32 i = 0; i < pNavMesh->GetNodes().size(); i++ )
    {
        m_AStarNodes.emplace_back();
        m_AStarNodes.back().pThisNode = pNavMesh->GetNode( i );
    }
}

CAStarSolver::~CAStarSolver( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    m_SortedByF.clear();
    m_AStarNodes.clear();
}

tgBool CAStarSolver::Search( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    if( m_pCurrentNode == m_pGoalNode )
        return true;

    if( m_pCurrentNode == m_pStartNode )
    {
        SAStarNode* pStartNode = &m_AStarNodes[m_pStartNode->Index];
        pStartNode->pThisNode  = m_pStartNode;
    }

    for( SNavMeshNode* pNeighbourNode : m_pCurrentNode->NeighbourNodes )
    {
        if( pNeighbourNode->IsClosed )
            continue;

        const SAStarNode* pCurrentAStarNode = &m_AStarNodes[m_pCurrentNode->Index];

        const tgFloat G = CalculateG( pCurrentAStarNode, pNeighbourNode );
        const tgFloat H = CalculateH( pNeighbourNode );
        const tgFloat F = G + H;

        SAStarNode* pNeighbourAStarNode = &m_AStarNodes[pNeighbourNode->Index];
        if( pNeighbourNode->IsVisited )
        {
            if( F < pNeighbourAStarNode->F )
            {
                pNeighbourNode->pParentNode = m_pCurrentNode;
                pNeighbourAStarNode->G      = G;
                pNeighbourAStarNode->H      = H;
                pNeighbourAStarNode->F      = F;
            }
        }
        else
        {
            pNeighbourNode->pParentNode = m_pCurrentNode;
            pNeighbourNode->IsVisited   = true;

            pNeighbourAStarNode->G = G;
            pNeighbourAStarNode->H = H;
            pNeighbourAStarNode->F = F;

            const auto it = std::lower_bound( m_SortedByF.begin(), m_SortedByF.end(), pNeighbourAStarNode, SortAscendingF );
            m_SortedByF.insert( it, pNeighbourAStarNode );
        }
    }

    if( m_SortedByF.empty() )
        return true;

    m_pCurrentNode           = m_SortedByF.front()->pThisNode;
    m_pCurrentNode->IsClosed = true;
    m_SortedByF.erase( m_SortedByF.begin() );

    return false;
}

void CAStarSolver::Clear( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    m_SortedByF.clear();

    for( SAStarNode& rNode : m_AStarNodes )
    {
        rNode.G = 0;
        rNode.H = 0;
        rNode.F = 0;

        rNode.pThisNode->pParentNode = nullptr;
        rNode.pThisNode->IsClosed    = false;
        rNode.pThisNode->IsVisited   = false;
    }
}

tgFloat CAStarSolver::CalculateG( const SAStarNode* pCurrentNode, const SNavMeshNode* pNeighbourNode )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const tgFloat X = pNeighbourNode->Center.x - pCurrentNode->pThisNode->Center.x;
    const tgFloat Y = pNeighbourNode->Center.y - pCurrentNode->pThisNode->Center.y;
    const tgFloat Z = pNeighbourNode->Center.z - pCurrentNode->pThisNode->Center.z;

    return pCurrentNode->G + ( X * X ) + ( Y * Y ) + ( Z * Z );
}

tgFloat CAStarSolver::CalculateH( const SNavMeshNode* pNeighbourNode )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const tgFloat X = m_pGoalNode->Center.x - pNeighbourNode->Center.x;
    const tgFloat Y = m_pGoalNode->Center.y - pNeighbourNode->Center.y;
    const tgFloat Z = m_pGoalNode->Center.z - pNeighbourNode->Center.z;

    return ( X * X ) + ( Y * Y ) + ( Z * Z );
}
