#pragma once

#include "SOctreeNode.h"

class COctree
{
public:
    COctree( tgUInt32 DepthLimit );

    void UpdateObject( IOctreeObject* pObject );

    void Render( void );

    void Insert( IOctreeObject* pObject, SOctreeNode* pCurrentNode = nullptr );

    std::vector<SOctreeNode*>  GetNodesWithObjects( void );
    std::vector<SOctreeNode*>& GetDeepestNodes( void ) { return m_DeepestNodes; }
    const SOctreeNode*         GetNode( const tgCV3D& rPoint, SOctreeNode* pCurrentNode = nullptr );

private:
    void   FindDeepestNodes( std::vector<SOctreeNode*>& rOutput, SOctreeNode* pCurrentNode = nullptr );
    void   CreateOctree( SOctreeNode* pCurrentNode = nullptr );
    tgBool CreateNode( SOctreeNode* pParentNode, const tgUInt32 OffsetIndex );

    void GetNeighbours( void );
    void UpdateObjectNeighbourNodes( IOctreeObject* pObject, const SOctreeNode* pCurrentNode );

    const tgUInt32 m_DepthLimit;

    SOctreeNode               m_RootNode;
    const std::vector<tgCV3D> m_Offsets;

    std::vector<SOctreeNode*> m_DeepestNodes;
};
