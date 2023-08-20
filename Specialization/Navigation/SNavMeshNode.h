#pragma once

#include <tgCTriangle3D.h>

#include <tgMemoryDisable.h>
#include <vector>
#include <tgMemoryEnable.h>

struct SNavMeshNode
{
#include <tgMemoryDisable.h>
    SNavMeshNode( SNavMeshNode& ) = delete;
#include <tgMemoryEnable.h>

    SNavMeshNode( SNavMeshNode&& ) = default;

    SNavMeshNode( void )
        : Triangle( 0, 0, 0 )
        , Center( 0 )
        , Normal( 0 )
        , Index( 0 )
        , IsVisited( false )
        , IsClosed( false )
        , pParentNode( nullptr )
        , NeighbourNodes()
    {}

    tgCTriangle3D Triangle;
    tgCV3D        Center;
    tgCV3D        Normal;

    tgSize Index;

    tgBool IsVisited;
    tgBool IsClosed;

    SNavMeshNode*              pParentNode;
    std::vector<SNavMeshNode*> NeighbourNodes;
};
