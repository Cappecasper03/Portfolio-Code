#include <tgSystem.h>

#include "COctree.h"
#include "IOctreeObject.h"
#include "Navigation/CNavMesh.h"
#include "Specialization/CLevel.h"

#include <tgCV3D.h>
#include <tgCDebugManager.h>
#include <tgCProfiling.h>
#include <tgMemoryDisable.h>
#include <algorithm>
#include <tgCLine3D.h>
#include <tgCSphere.h>
#include <tgMemoryEnable.h>

tgBool SortAscendingId( const IOctreeObject* pLIn, const IOctreeObject* pRIn ) { return pLIn->GetId() < pRIn->GetId(); }

COctree::COctree( const tgUInt32 DepthLimit )
    : m_DepthLimit( DepthLimit )
    , m_RootNode()
    , m_Offsets{ tgCV3D( -1 ) ,tgCV3D( -1, -1, 1 ) ,tgCV3D( -1, 1, -1 ) ,tgCV3D( -1, 1, 1 ) ,tgCV3D( 1, -1, -1 ) ,tgCV3D( 1, -1, 1 ) ,tgCV3D( 1, 1, -1 ) ,tgCV3D( 1 ) }
    , m_DeepestNodes()
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif

    CNavMesh* pNavMesh = CLevel::GetInstance().GetNavMesh();

    m_RootNode.Box.Set( pNavMesh->GetNode( 0 )->Center );
    for( SNavMeshNode& rNavMeshNode : pNavMesh->GetNodes() )
    {
        for( int i = 0; i < 3; ++i )
            m_RootNode.Box.AddPoint( rNavMeshNode.Triangle.GetVertex( i ) );
    }

    const tgCV3D& NavBoxMin = m_RootNode.Box.GetMin();
    const tgCV3D& NavBoxMax = m_RootNode.Box.GetMax();
    m_RootNode.Center       = ( NavBoxMax - NavBoxMin ) / 2 + NavBoxMin;

    tgFloat CubeExtent = tgMathAbs( NavBoxMin.x - m_RootNode.Center.x );
    CubeExtent         = tgMathAbs( NavBoxMin.y - m_RootNode.Center.y ) > CubeExtent ? tgMathAbs( NavBoxMin.y - m_RootNode.Center.y ) : CubeExtent;
    CubeExtent         = tgMathAbs( NavBoxMin.z - m_RootNode.Center.z ) > CubeExtent ? tgMathAbs( NavBoxMin.z - m_RootNode.Center.z ) : CubeExtent;
    CubeExtent         = NavBoxMax.x - m_RootNode.Center.x > CubeExtent ? NavBoxMax.x - m_RootNode.Center.x : CubeExtent;
    CubeExtent         = NavBoxMax.y - m_RootNode.Center.y > CubeExtent ? NavBoxMax.y - m_RootNode.Center.y : CubeExtent;
    CubeExtent         = NavBoxMax.z - m_RootNode.Center.z > CubeExtent ? NavBoxMax.z - m_RootNode.Center.z : CubeExtent;

    m_RootNode.Box.Set( m_RootNode.Center - CubeExtent, m_RootNode.Center + CubeExtent );
    m_RootNode.pParentNode = nullptr;
    m_RootNode.OffsetIndex = -1;
    m_RootNode.DepthIndex  = 0;

    CreateOctree();
    FindDeepestNodes( m_DeepestNodes );
    GetNeighbours();
}

void COctree::UpdateObject( IOctreeObject* pObject )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    if( !pObject )
        return;

    SOctreeNode* pCurrentNode = pObject->GetCurrentNode();

    if( pCurrentNode )
    {
        const tgFloat    BoxExtent = pObject->GetBoundingSphere()->GetRadius();
        const tgCAABox3D ObjectBox( *pObject->GetPosition() - BoxExtent, *pObject->GetPosition() + BoxExtent );

        if( !pCurrentNode->Box.PointInside( *pObject->GetPosition() ) )
        {
            const auto it = std::lower_bound( pCurrentNode->Objects.begin(), pCurrentNode->Objects.end(), pObject, SortAscendingId );
            if( it._Ptr && *it._Ptr == pObject && !pCurrentNode->Objects.empty() )
            {
                pCurrentNode->Objects.erase( it );
                pObject->SetCurrentNode( nullptr );
            }

            Insert( pObject );
        }

        UpdateObjectNeighbourNodes( pObject, pCurrentNode );
    }
    else
        Insert( pObject );
}

void COctree::Render( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgCDebugManager&                rDebugManager = tgCDebugManager::GetInstance();
    const std::vector<SOctreeNode*> Nodes         = GetNodesWithObjects();

    for( const SOctreeNode* pNode : Nodes )
    {
        rDebugManager.AddLineAABox3D( pNode->Box, tgCColor::Green );

        for( const SOctreeNode* pNeighbourNode : pNode->NeighbourNodes )
            rDebugManager.AddLine3D( tgCLine3D( pNeighbourNode->Center, pNode->Center ), tgCColor::Gray );
    }
}

void COctree::Insert( IOctreeObject* pObject, SOctreeNode* pCurrentNode )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    if( !pCurrentNode )
    {
        pCurrentNode = &m_RootNode;

        if( !pCurrentNode->Box.PointInside( *pObject->GetPosition() ) )
            return;
    }

    if( pCurrentNode->ChildNodes.empty() )
    {
        const auto it = std::lower_bound( pCurrentNode->Objects.begin(), pCurrentNode->Objects.end(), pObject, SortAscendingId );
        if( it._Ptr && *it._Ptr == pObject && !pCurrentNode->Objects.empty() )
            return;

        pObject->SetCurrentNode( pCurrentNode );
        pCurrentNode->Objects.insert( it, pObject );
        return;
    }

    for( SOctreeNode& rNode : pCurrentNode->ChildNodes )
    {
        if( rNode.Box.PointInside( *pObject->GetPosition() ) )
            Insert( pObject, &rNode );
    }
}

std::vector<SOctreeNode*> COctree::GetNodesWithObjects( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    std::vector<SOctreeNode*> NodesWithObjects;
    for( SOctreeNode* pNode : m_DeepestNodes )
    {
        if( !pNode->Objects.empty() )
            NodesWithObjects.push_back( pNode );
    }

    return std::move( NodesWithObjects );
}

const SOctreeNode* COctree::GetNode( const tgCV3D& rPoint, SOctreeNode* pCurrentNode )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    if( !pCurrentNode )
        pCurrentNode = &m_RootNode;

    for( SOctreeNode& rNode : pCurrentNode->ChildNodes )
    {
        if( rNode.Box.PointInside( rPoint ) )
            return GetNode( rPoint, &rNode );
    }

    return nullptr;
}

void COctree::FindDeepestNodes( std::vector<SOctreeNode*>& rOutput, SOctreeNode* pCurrentNode )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    if( !pCurrentNode )
        pCurrentNode = &m_RootNode;

    for( SOctreeNode& rNode : pCurrentNode->ChildNodes )
        FindDeepestNodes( rOutput, &rNode );

    if( pCurrentNode->ChildNodes.empty() )
        rOutput.push_back( pCurrentNode );
}

void COctree::CreateOctree( SOctreeNode* pCurrentNode )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    if( !pCurrentNode )
        pCurrentNode = &m_RootNode;

    if( pCurrentNode->DepthIndex + 1 >= m_DepthLimit )
        return;

    for( tgUInt32 i = 0; i < 8; i++ )
    {
        if( CreateNode( pCurrentNode, i ) )
            CreateOctree( &pCurrentNode->ChildNodes.back() );
    }
}

tgBool COctree::CreateNode( SOctreeNode* pParentNode, const tgUInt32 OffsetIndex )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const tgCV3D& rMinPoint = pParentNode->Box.GetMin();
    const tgCV3D& rMaxPoint = pParentNode->Box.GetMax();
    const tgCV3D  BoxExtent = ( rMaxPoint - rMinPoint ) / 4;

    SOctreeNode Node{};
    Node.Center = pParentNode->Center + ( m_Offsets[OffsetIndex] * BoxExtent );
    Node.Box.Set( Node.Center + -BoxExtent, Node.Center + BoxExtent );

    Node.pParentNode = pParentNode;
    Node.OffsetIndex = OffsetIndex;
    Node.DepthIndex  = pParentNode->DepthIndex + 1;

    tgCAABox3D Box( 0, 0 );

    for( SNavMeshNode& rNavMeshNode : CLevel::GetInstance().GetNavMesh()->GetNodes() )
    {
        Box.Set( rNavMeshNode.Center - 1, rNavMeshNode.Center + 1 );
        if( Node.Box.Intersect( Box ) )
        {
            pParentNode->ChildNodes.push_back( std::move( Node ) );
            return true;
        }
    }

    return false;
}

void COctree::GetNeighbours( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    for( tgUInt32 NodeIndex = 0; NodeIndex < m_DeepestNodes.size(); NodeIndex++ )
    {
        SOctreeNode* pNode = m_DeepestNodes[NodeIndex];

        for( tgUInt32 OtherNodeIndex = NodeIndex + 1; OtherNodeIndex < m_DeepestNodes.size(); OtherNodeIndex++ )
        {
            SOctreeNode* pOtherNode = m_DeepestNodes[OtherNodeIndex];

            tgBool AlreadyNeighbours = false;
            for( const SOctreeNode* pNeighbourNode : pNode->NeighbourNodes )
            {
                if( pOtherNode == pNeighbourNode )
                {
                    AlreadyNeighbours = true;
                    break;
                }
            }

            if( AlreadyNeighbours )
                continue;

            if( pNode->Box.Intersect( pOtherNode->Box ) )
            {
                pNode->NeighbourNodes.push_back( pOtherNode );
                pOtherNode->NeighbourNodes.push_back( pNode );
            }
        }
    }
}

void COctree::UpdateObjectNeighbourNodes( IOctreeObject* pObject, const SOctreeNode* pCurrentNode )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const tgFloat              BoxExtent = pObject->GetBoundingSphere()->GetRadius();
    const tgCAABox3D           ObjectBox( *pObject->GetPosition() - BoxExtent, *pObject->GetPosition() + BoxExtent );
    std::vector<SOctreeNode*>& rObjectNeighbourNodes = pObject->GetCurrentNeighbourNodes();
    rObjectNeighbourNodes.clear();

    for( SOctreeNode* pNeighbourNode : pCurrentNode->NeighbourNodes )
    {
        if( pNeighbourNode->Box.Intersect( ObjectBox ) )
            rObjectNeighbourNodes.push_back( pNeighbourNode );
    }
}
