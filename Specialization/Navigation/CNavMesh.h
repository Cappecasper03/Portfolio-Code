#pragma once

#include "SNavMeshNode.h"

#include <tgMemoryDisable.h>
#include <vector>
#include <tgMemoryEnable.h>

class CNavMesh
{
public:
    CNavMesh( const tgCString& rWorldName );
    ~CNavMesh( void );

    SNavMeshNode*              GetNode( const tgUInt32 Index ) { return &m_Nodes[Index]; }
    SNavMeshNode*              GetNode( const tgCV3D& rPoint );
    std::vector<SNavMeshNode>& GetNodes( void ) { return m_Nodes; }

    std::vector<tgCLine3D>& GetEdges( void ) { return m_Edges; }

    void Render();

private:
    void LoopSectorMeshes( const tgSWorldSector* pSector );
    void LoopMeshIndices( const tgCMesh* pMesh );
    void CreateNode( const tgCMesh* pMesh, const tgUInt32 IndiceIndex );

    void FindNeighbours( void );
    void FindNeighbours( SNavMeshNode& rNode );
    void FindNeighbours( SNavMeshNode& rNode, SNavMeshNode& rNeighbourNode );

    void                       FindEdges( void );
    void                       CombineEdges( std::vector<tgCLine3D>& rEdges );
    std::vector<const tgCV3D*> GetSharedVertices( const SNavMeshNode* pNode1, const SNavMeshNode* pNode2 );

    void CreateNodes( void );

    std::vector<SNavMeshNode> m_Nodes;
    std::vector<tgCLine3D>    m_Edges;

    tgCWorld* m_pWorld;
};
