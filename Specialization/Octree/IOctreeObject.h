#pragma once

#include <tgSystem.h>

#include <tgCV3D.h>

#include <tgMemoryDisable.h>
#include <vector>
#include <tgMemoryEnable.h>

struct SOctreeNode;

class IOctreeObject
{
public:
    IOctreeObject( const tgSize Id, const tgCV3D* pPosition, const tgCSphere* pBoundingSphere )
        : m_Id( Id )
        , m_pPosition( pPosition )
        , m_pBoundingSphere( pBoundingSphere )
        , m_pCurrentNode( nullptr )
        , m_CurrentNeighbourNodes()
    {}

    const tgSize& GetId( void ) const { return m_Id; }

    const tgCV3D*    GetPosition( void ) { return m_pPosition; }
    const tgCSphere* GetBoundingSphere( void ) { return m_pBoundingSphere; }

    SOctreeNode* GetCurrentNode( void ) { return m_pCurrentNode; }
    void         SetCurrentNode( SOctreeNode* pCurrentNode ) { m_pCurrentNode = pCurrentNode; }

    std::vector<SOctreeNode*>& GetCurrentNeighbourNodes( void ) { return m_CurrentNeighbourNodes; }

protected:
    const tgSize m_Id;

    const tgCV3D*    m_pPosition;
    const tgCSphere* m_pBoundingSphere;

    SOctreeNode*              m_pCurrentNode;
    std::vector<SOctreeNode*> m_CurrentNeighbourNodes;
};
