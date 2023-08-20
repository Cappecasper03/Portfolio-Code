#pragma once

#include <d3d11.h>
#include <tgCMatrix.h>

#include <tgMemoryDisable.h>
#include <vector>
#include <tgMemoryEnable.h>

class CEnemy;

class CEnemyManager
{
public:
    struct SMeshInstanceData
    {
        tgCMatrix Matrix;
    };

    struct SModelInstance
    {
        ID3D11Buffer*            pInstanceBuffer;
        D3D11_MAPPED_SUBRESOURCE SubResourceMeshInstanceData;
        tgUInt32                 NumMeshes;
    };

    CEnemyManager( void );
    ~CEnemyManager( void );

    void Update( const tgFloat DeltaTime );
    void UpdateEnemyPaths();
    void UpdateEnemies( tgFloat DeltaTime );
    void UpdateDeadEnemy( CEnemy* pEnemy );

    void Render( const tgBool DoDebugRender );

    tgSize          GetNumEnemies( void ) { return m_Enemies.size(); }
    const tgUInt32& GetNumRenderedEnemies( void ) { return m_ModelInstance.NumMeshes; }

protected:
    std::vector<CEnemy*> m_Enemies;
    const tgUInt32       m_AmountOfEnemies;

    tgCModel*      m_pEnemyModel;
    SModelInstance m_ModelInstance;

    tgFloat m_MaxDistanceToTargetPlayer;
};
