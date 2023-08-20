#pragma once

#include <tgCAABox3D.h>

#include <tgMemoryDisable.h>
#include <vector>
#include <tgMemoryEnable.h>

class IOctreeObject;

struct SOctreeNode
{
#include <tgMemoryDisable.h>
    SOctreeNode( const SOctreeNode& ) = delete;
#include <tgMemoryEnable.h>

    SOctreeNode( SOctreeNode&& ) = default;

    SOctreeNode( void )
        : Box( 0 )
        , Center( 0 )
        , pParentNode( nullptr )
        , ChildNodes()
        , NeighbourNodes()
        , OffsetIndex( -1 )
        , DepthIndex( 0 )
        , Objects()
    {}

    tgCAABox3D Box;
    tgCV3D     Center;

    SOctreeNode*              pParentNode;
    std::vector<SOctreeNode>  ChildNodes;
    std::vector<SOctreeNode*> NeighbourNodes;

    tgSInt32 OffsetIndex;
    tgUInt32 DepthIndex;

    std::vector<IOctreeObject*> Objects;
};
