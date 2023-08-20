#pragma once

#include "Navigation/SNavMeshNode.h"

typedef float tgFloat;

struct SAStarNode
{
#include <tgMemoryDisable.h>
    SAStarNode( SAStarNode& ) = delete;
#include <tgMemoryEnable.h>

    SAStarNode( SAStarNode&& ) = default;

    SAStarNode( void )
        : pThisNode( nullptr )
        , G( 0 )
        , H( 0 )
        , F( 0 )
    { }

    SNavMeshNode* pThisNode;

    tgFloat G;
    tgFloat H;
    tgFloat F;
};
