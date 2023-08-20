#include <tgSystem.h>

#include "CNavMesh.h"
#include "Managers/CWorldManager.h"

#include <tgCProfiling.h>
#include <tgCV3D.h>
#include <tgCDebugManager.h>
#include <tgCLine3D.h>

CNavMesh::CNavMesh( const tgCString& rWorldName )
    : m_Nodes()
    , m_Edges()
    , m_pWorld( nullptr )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif

    m_pWorld = CWorldManager::GetInstance().GetWorld( rWorldName );
    if( !m_pWorld )
        return;

    CreateNodes();
    FindNeighbours();
    FindEdges();
}

CNavMesh::~CNavMesh( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif

    m_Nodes.clear();
}

SNavMeshNode* CNavMesh::GetNode( const tgCV3D& rPoint )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const tgCLine3D Line( rPoint + tgCV3D( 0, 1, 0 ), rPoint - tgCV3D( 0, 10, 0 ) );

    for( SNavMeshNode& rNode : m_Nodes )
    {
        if( Line.Intersect( rNode.Triangle ) )
            return &rNode;
    }

    return nullptr;
}

void CNavMesh::CreateNodes( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    for( tgUInt32 SectorIndex = 0; SectorIndex < m_pWorld->GetNumSectors(); SectorIndex++ )
        LoopSectorMeshes( m_pWorld->GetSector( SectorIndex ) );
}

void CNavMesh::Render()
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgCDebugManager& rDebugManager = tgCDebugManager::GetInstance();

    for( SNavMeshNode& rNode : m_Nodes )
    {
        tgCTriangle3D Triangle = rNode.Triangle;
        for( int i = 0; i < 3; ++i )
        {
            const tgCV3D& Vertex            = rNode.Triangle.GetVertex( i );
            const tgCV3D  VertexToCenterDir = ( rNode.Center - Vertex ).Normalized();

            Triangle.GetVertex( i ) += VertexToCenterDir * .01f + rNode.Normal * .01f;
        }

        rDebugManager.AddTriangle3D( Triangle, tgCColor::Purple );
    }

    for( tgCLine3D& rEdge : m_Edges )
    {
        tgCLine3D Line( rEdge.GetStart() + tgCV3D( 0, .01f, 0 ), rEdge.GetEnd() + tgCV3D( 0, .01f, 0 ) );

        rDebugManager.AddLine3D( Line, tgCColor::Lime );
    }
}

void CNavMesh::LoopSectorMeshes( const tgSWorldSector* pSector )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    for( tgUInt32 MeshIndex = 0; MeshIndex < pSector->NumMeshes; MeshIndex++ )
        LoopMeshIndices( &pSector->pMeshArray[MeshIndex] );
}

void CNavMesh::LoopMeshIndices( const tgCMesh* pMesh )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    for( tgUInt32 IndiceIndex = 0; IndiceIndex < pMesh->GetNumTotalIndices(); IndiceIndex += 3 )
        CreateNode( pMesh, IndiceIndex );
}

void CNavMesh::CreateNode( const tgCMesh* pMesh, const tgUInt32 IndiceIndex )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const tgCMesh::SVertex* pVertex0 = pMesh->GetVertex( pMesh->GetIndex( IndiceIndex ) );
    const tgCMesh::SVertex* pVertex1 = pMesh->GetVertex( pMesh->GetIndex( IndiceIndex + 1 ) );
    const tgCMesh::SVertex* pVertex2 = pMesh->GetVertex( pMesh->GetIndex( IndiceIndex + 2 ) );

    SNavMeshNode Node{};
    Node.Triangle = tgCTriangle3D( pVertex0->Position, pVertex1->Position, pVertex2->Position );

    const tgCV3D* VertexArray = Node.Triangle.GetVertexArray();
    Node.Center.x             = ( VertexArray[0].x + VertexArray[1].x + VertexArray[2].x ) / 3;
    Node.Center.y             = ( VertexArray[0].y + VertexArray[1].y + VertexArray[2].y ) / 3;
    Node.Center.z             = ( VertexArray[0].z + VertexArray[1].z + VertexArray[2].z ) / 3;

    Node.Normal = ( pVertex0->Normal + pVertex1->Normal + pVertex2->Normal ) / 3;

    Node.Index = m_Nodes.size();

    Node.IsVisited = false;
    Node.IsClosed  = false;

    m_Nodes.push_back( std::move( Node ) );
}

void CNavMesh::FindNeighbours( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    for( SNavMeshNode& rNode : m_Nodes )
        FindNeighbours( rNode );
}

void CNavMesh::FindNeighbours( SNavMeshNode& rNode )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    for( tgSize i = rNode.Index + 1; i < m_Nodes.size(); ++i )
    {
        SNavMeshNode& rNeighbourNode = m_Nodes[i];

        if( rNode.NeighbourNodes.size() >= 3 )
            break;

        if( &rNode == &rNeighbourNode )
            continue;

        FindNeighbours( rNode, rNeighbourNode );
    }
}

void CNavMesh::FindNeighbours( SNavMeshNode& rNode, SNavMeshNode& rNeighbourNode )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgUInt32 SharedVertices = 0;
    for( tgUInt32 NodeVertexIndex = 0; NodeVertexIndex < 3; NodeVertexIndex++ )
    {
        tgCV3D& rNodeVertex = rNode.Triangle.GetVertex( NodeVertexIndex );

        for( tgUInt32 NeighbourVertexIndex = 0; NeighbourVertexIndex < 3; NeighbourVertexIndex++ )
        {
            tgCV3D& rNeighbourVertex = rNeighbourNode.Triangle.GetVertex( NeighbourVertexIndex );

            if( rNodeVertex == rNeighbourVertex )
            {
                SharedVertices++;
                break;
            }
        }

        if( SharedVertices == 2 )
        {
            rNode.NeighbourNodes.push_back( &rNeighbourNode );
            rNeighbourNode.NeighbourNodes.push_back( &rNode );
            break;
        }
    }
}

void CNavMesh::FindEdges( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    std::vector<tgCLine3D> Edges;

    for( SNavMeshNode& rNode : m_Nodes )
    {
        std::vector<const tgCV3D*>  pSharedVertices;
        std::vector<SNavMeshNode*>& rNeighbourNodes = rNode.NeighbourNodes;

        if( rNeighbourNodes.size() == 1 )
            pSharedVertices = GetSharedVertices( &rNode, rNeighbourNodes[0] );
        else if( rNeighbourNodes.size() == 2 )
            pSharedVertices = GetSharedVertices( rNeighbourNodes[0], rNeighbourNodes[1] );
        else
            continue;

        const tgCV3D*              VertexArray  = rNode.Triangle.GetVertexArray();
        std::vector<const tgCV3D*> NodeVertices = { &VertexArray[0] ,&VertexArray[1] ,&VertexArray[2] };
        tgCLine3D                  Edge( 0 );

        for( const tgCV3D* pNodeVertex : NodeVertices )
        {
            tgBool SharedVertex = false;
            for( const tgCV3D* pSharedVertex : pSharedVertices )
            {
                if( *pSharedVertex == *pNodeVertex )
                {
                    SharedVertex = true;
                    break;
                }
            }

            if( !SharedVertex )
            {
                if( rNeighbourNodes.size() == 1 )
                {
                    Edges.emplace_back( *pNodeVertex, *pSharedVertices[0] );
                    Edges.emplace_back( *pNodeVertex, *pSharedVertices[1] );
                    break;
                }

                if( rNeighbourNodes.size() == 2 )
                {
                    if( Edge.GetStart() == 0 )
                        Edge.SetStart( *pNodeVertex );
                    else
                    {
                        Edge.SetEnd( *pNodeVertex );
                        Edges.push_back( Edge );
                        break;
                    }
                }
            }
        }
    }

    CombineEdges( Edges );
}

void CNavMesh::CombineEdges( std::vector<tgCLine3D>& rEdges )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    while( !rEdges.empty() )
    {
        tgBool HasCombinedEdges = false;
        m_Edges.push_back( rEdges.back() );
        rEdges.pop_back();

        do
        {
            HasCombinedEdges          = false;
            tgCLine3D& rEdge1         = m_Edges.back();
            tgCV3D     StartToEndDir1 = ( rEdge1.GetEnd() - rEdge1.GetStart() ).Normalized();

            for( tgUInt32 i = 0; i < rEdges.size(); ++i )
            {
                tgCLine3D&    rEdge2 = rEdges[i];
                tgCV3D        EdgeDir( 0 );
                const tgCV3D* pEdgePoint1 = nullptr;
                const tgCV3D* pEdgePoint2 = nullptr;

                if( rEdge1.GetStart() == rEdge2.GetStart() )
                {
                    EdgeDir     = ( rEdge2.GetStart() - rEdge2.GetEnd() ).Normalized();
                    pEdgePoint1 = &rEdge1.GetStart();
                    pEdgePoint2 = &rEdge2.GetEnd();
                }
                else if( rEdge1.GetStart() == rEdge2.GetEnd() )
                {
                    EdgeDir     = ( rEdge2.GetEnd() - rEdge2.GetStart() ).Normalized();
                    pEdgePoint1 = &rEdge1.GetStart();
                    pEdgePoint2 = &rEdge2.GetStart();
                }
                else if( rEdge1.GetEnd() == rEdge2.GetEnd() )
                {
                    EdgeDir     = ( rEdge2.GetStart() - rEdge2.GetEnd() ).Normalized();
                    pEdgePoint1 = &rEdge1.GetEnd();
                    pEdgePoint2 = &rEdge2.GetStart();
                }
                else if( rEdge1.GetEnd() == rEdge2.GetStart() )
                {
                    EdgeDir     = ( rEdge2.GetEnd() - rEdge2.GetStart() ).Normalized();
                    pEdgePoint1 = &rEdge1.GetEnd();
                    pEdgePoint2 = &rEdge2.GetEnd();
                }

                if( pEdgePoint1 && pEdgePoint2 && EdgeDir.Between( StartToEndDir1 - .1f, StartToEndDir1 + .1f ) )
                {
                    rEdge1.SetEnd( rEdge2.GetEnd() );
                    rEdges.erase( rEdges.begin() + i );
                    HasCombinedEdges = true;
                    break;
                }
            }
        } while( HasCombinedEdges );
    }
}

std::vector<const tgCV3D*> CNavMesh::GetSharedVertices( const SNavMeshNode* pNode1, const SNavMeshNode* pNode2 )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    std::vector<const tgCV3D*> SharedVertices;
    for( tgUInt32 Node1VertexIndex = 0; Node1VertexIndex < 3; Node1VertexIndex++ )
    {
        const tgCV3D& rNode1Vertex = pNode1->Triangle.GetVertex( Node1VertexIndex );

        for( tgUInt32 Node2VertexIndex = 0; Node2VertexIndex < 3; Node2VertexIndex++ )
        {
            const tgCV3D& rNode2Vertex = pNode2->Triangle.GetVertex( Node2VertexIndex );

            if( rNode1Vertex == rNode2Vertex )
                SharedVertices.push_back( &rNode1Vertex );
        }
    }

    return std::move( SharedVertices );
}
