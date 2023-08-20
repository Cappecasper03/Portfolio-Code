#pragma once

#include "CSolver.h"
#include "../SAStarNode.h"

class CAStarSolver : public CSolver
{
public:
    CAStarSolver( CNavMesh* pNavMesh, tgCMutex* pMutex );
    ~CAStarSolver( void ) override;

private:
    tgBool Search( void ) override;

    void Clear( void ) override;

    tgFloat CalculateG( const SAStarNode* pCurrentNode, const SNavMeshNode* pNeighbourNode );
    tgFloat CalculateH( const SNavMeshNode* pNeighbourNode );

    std::vector<SAStarNode*> m_SortedByF;
    std::vector<SAStarNode>  m_AStarNodes;
};
