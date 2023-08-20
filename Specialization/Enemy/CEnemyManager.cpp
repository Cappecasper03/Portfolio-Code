#include <tgSystem.h>

#include "CEnemyManager.h"
#include "CEnemy.h"
#include "Managers/CModelManager.h"
#include "Navigation/CNavMesh.h"
#include "Managers/CWorldManager.h"
#include "Octree/COctree.h"
#include "Navigation/Pathfinding/CPathfindingManager.h"
#include "Renderer/CRenderCallBacks.h"
#include "Specialization/CPlayer.h"
#include "Specialization/CLevel.h"

#include <tgCCollision.h>
#include <tgCLightManager.h>
#include <tgCLine3D.h>
#include <tgCProfiling.h>
#include <tgCCameraManager.h>
#include <tgFrustum.h>
#include <tgError.h>

CEnemyManager::CEnemyManager()
    : m_Enemies()
    , m_AmountOfEnemies( 100 )
    , m_pEnemyModel( nullptr )
    , m_ModelInstance{}
    , m_MaxDistanceToTargetPlayer( 5 )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    CModelManager& rModelManager = CModelManager::GetInstance();
    m_pEnemyModel                = rModelManager.LoadModel( "models/droid_big", "Enemy", false );

    tgCSphere ModelBoundingSphere( m_pEnemyModel->GetTransform().GetMatrixWorld().Pos, 0 );
    for( tgUInt32 i = 0; i < m_pEnemyModel->GetNumMeshes(); ++i )
        ModelBoundingSphere.AddSphere( m_pEnemyModel->GetMesh( i )->GetBSphere() );


    const CLevel& rLevel          = CLevel::GetInstance();
    const tgCV3D& rPlayerLocation = rLevel.GetPlayer()->GetPosition();
    CNavMesh*     pNavMesh        = rLevel.GetNavMesh();

    for( tgUInt32 i = 0; m_Enemies.size() < m_AmountOfEnemies; ++i )
    {
        tgCMatrix RotationMatrix;
        RotationMatrix.RotateY( tgMathRandom( -180.f, 180.f ), tgCMatrix::COMBINE_REPLACE );
        tgCV3D RandomStartPos = pNavMesh->GetNode( tgMathRandom( 0, static_cast<tgSInt32>( rLevel.GetNavMesh()->GetNodes().size() ) - 1 ) )->Center;
        RandomStartPos += RotationMatrix.At * tgMathRandom( 0.f, 5.f );

        if( ( RandomStartPos - rPlayerLocation ).Length() < 25 )
            continue;

        tgCLine3D    Line( tgCV3D( RandomStartPos.x, RandomStartPos.y + 2, RandomStartPos.z ), tgCV3D( RandomStartPos.x, RandomStartPos.y - 5, RandomStartPos.z ) );
        tgCCollision Collision( true );
        Collision.SetType( tgCMesh::TYPE_WORLD );
        if( pNavMesh->GetNode( RandomStartPos ) && Collision.LineAllMeshesInWorld( Line, *rLevel.GetCollisionWorld() ) )
        {
            m_Enemies.push_back( new CEnemy( i, Collision.GetLocalIntersection(), ModelBoundingSphere ) );
            rLevel.GetOctree()->Insert( m_Enemies.back() );
        }
    }

    for( CPathfindingManager::SPathInfo& rPathInfo : rLevel.GetPathfindingManager()->GetPaths() )
        rPathInfo.GoalPosition = rPlayerLocation;

    tgCD3D11&     rD3D11  = tgCD3D11::GetInstance();
    ID3D11Device* pDevice = rD3D11.LockDevice();

    D3D11_BUFFER_DESC InstanceDesc{};
    InstanceDesc.ByteWidth           = sizeof( SMeshInstanceData ) * static_cast<tgUInt32>( m_Enemies.size() );
    InstanceDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    InstanceDesc.Usage               = D3D11_USAGE_DYNAMIC;
    InstanceDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    InstanceDesc.MiscFlags           = 0;
    InstanceDesc.StructureByteStride = 0;

    m_ModelInstance.NumMeshes                              = 0;
    m_ModelInstance.SubResourceMeshInstanceData.pData      = nullptr;
    m_ModelInstance.SubResourceMeshInstanceData.DepthPitch = 0;
    m_ModelInstance.SubResourceMeshInstanceData.RowPitch   = 0;

    TG_D3D11_CHECK_ERROR( pDevice->CreateBuffer( &InstanceDesc, nullptr, &m_ModelInstance.pInstanceBuffer ) );
    rD3D11.UnlockDevice();
}

CEnemyManager::~CEnemyManager( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    if( m_ModelInstance.pInstanceBuffer )
    {
        m_ModelInstance.pInstanceBuffer->Release();
        m_ModelInstance.pInstanceBuffer = nullptr;
    }

    for( const CEnemy* pEnemy : m_Enemies )
        delete pEnemy;
    m_Enemies.clear();

    if( m_pEnemyModel )
        CModelManager::GetInstance().DestroyModel( m_pEnemyModel );
}

void CEnemyManager::Update( const tgFloat DeltaTime )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    UpdateEnemyPaths();

    UpdateEnemies( DeltaTime );
}

void CEnemyManager::UpdateEnemyPaths()
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const CLevel&        rLevel              = CLevel::GetInstance();
    CPathfindingManager* pPathfindingManager = rLevel.GetPathfindingManager();
    const tgCV3D&        rPlayerLocation     = rLevel.GetPlayer()->GetPosition();

    for( CPathfindingManager::SPathInfo& rPathInfo : pPathfindingManager->GetPaths() )
    {
        rPathInfo.GoalPosition = rPlayerLocation;

        for( IOctreeObject* pOctreeObject : rPathInfo.pOctreeStartNode->Objects )
        {
            if( rPathInfo.SharedPath && rPathInfo.WeakPath.expired() )
                static_cast<CEnemy*>( pOctreeObject )->SetPath( *rPathInfo.SharedPath );
            else if( rPathInfo.WeakPath.lock() )
                static_cast<CEnemy*>( pOctreeObject )->SetPath( *rPathInfo.WeakPath.lock() );
        }
    }

    if( m_Enemies.size() > 1 )
        return;

    tgCMutexScopeLock               ScopeLock( pPathfindingManager->GetMutex() );
    CPathfindingManager::SPathInfo& rPathInfo = pPathfindingManager->GetCurrentPath();
    rPathInfo.GoalPosition                    = rPlayerLocation;

    if( rPathInfo.pOctreeStartNode )
    {
        for( IOctreeObject* pOctreeObject : rPathInfo.pOctreeStartNode->Objects )
        {
            if( rPathInfo.SharedPath && rPathInfo.WeakPath.expired() )
                static_cast<CEnemy*>( pOctreeObject )->SetPath( *rPathInfo.SharedPath );
            else if( rPathInfo.WeakPath.lock() )
                static_cast<CEnemy*>( pOctreeObject )->SetPath( *rPathInfo.WeakPath.lock() );
        }
    }
}

void CEnemyManager::UpdateEnemies( const tgFloat DeltaTime )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgCD3D11&            rD3D11         = tgCD3D11::GetInstance();
    ID3D11DeviceContext* pDeviceContext = rD3D11.LockDeviceContext();

    pDeviceContext->Map( m_ModelInstance.pInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_ModelInstance.SubResourceMeshInstanceData );
    rD3D11.UnlockDeviceContext();

    const CLevel&     rLevel          = CLevel::GetInstance();
    const tgCV3D&     rPlayerLocation = rLevel.GetPlayer()->GetPosition();
    COctree*          pOctree         = rLevel.GetOctree();
    const tgCPlane3D* pCameraFrustum  = tgCCameraManager::GetInstance().GetCurrentCamera()->GetFrustum();
    m_ModelInstance.NumMeshes         = 0;

    for( CEnemy* pEnemy : m_Enemies )
    {
        if( pEnemy->IsDead() )
            UpdateDeadEnemy( pEnemy );

        if( ( rPlayerLocation - *pEnemy->GetPosition() ).Length() < m_MaxDistanceToTargetPlayer )
        {
            pEnemy->SetTargetPoint( rPlayerLocation );
            pEnemy->Update( DeltaTime, false );
        }
        else
            pEnemy->Update( DeltaTime, true );

        pOctree->UpdateObject( pEnemy );

        if( tgFrustumTestSphere( pCameraFrustum, 5, pEnemy->GetBoundingSphere() ) )
        {
            SMeshInstanceData* pMeshData                = static_cast<SMeshInstanceData*>( m_ModelInstance.SubResourceMeshInstanceData.pData );
            pMeshData[m_ModelInstance.NumMeshes].Matrix = pEnemy->GetTransformMatrix();
            m_ModelInstance.NumMeshes++;
        }
    }

    pDeviceContext->Unmap( m_ModelInstance.pInstanceBuffer, 0 );
}

void CEnemyManager::UpdateDeadEnemy( CEnemy* pEnemy )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const CLevel& rLevel          = CLevel::GetInstance();
    const tgCV3D& rPlayerLocation = rLevel.GetPlayer()->GetPosition();
    CNavMesh*     pNavMesh        = rLevel.GetNavMesh();
    tgBool        UpdatedEnemy    = false;

    do
    {
        tgCV3D RandomStartPos = pNavMesh->GetNode( tgMathRandom( 0, static_cast<tgSInt32>( rLevel.GetNavMesh()->GetNodes().size() ) - 1 ) )->Center;

        do
        {
            tgCMatrix RotationMatrix;
            RotationMatrix.RotateY( tgMathRandom( -180.f, 180.f ), tgCMatrix::COMBINE_REPLACE );
            RandomStartPos += RotationMatrix.At * tgMathRandom( 0.f, 5.f );
        } while( ( RandomStartPos - rPlayerLocation ).Length() < 25 );

        const tgCLine3D Line( tgCV3D( RandomStartPos.x, RandomStartPos.y + 2, RandomStartPos.z ), tgCV3D( RandomStartPos.x, RandomStartPos.y - 5, RandomStartPos.z ) );
        tgCCollision    Collision( true );
        Collision.SetType( tgCMesh::TYPE_WORLD );
        if( pNavMesh->GetNode( RandomStartPos ) && Collision.LineAllMeshesInWorld( Line, *rLevel.GetCollisionWorld() ) )
        {
            pEnemy->SetPosition( Collision.GetLocalIntersection() );
            rLevel.GetOctree()->UpdateObject( pEnemy );
            UpdatedEnemy = true;
        }
    } while( !UpdatedEnemy );

    pEnemy->SetDead( false );
}

void CEnemyManager::Render( const tgBool DoDebugRender )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgCLightManager&                  rLightManager = tgCLightManager::GetInstance();
    const tgCLightManager::LightList& rLightList    = rLightManager.GetLightList();

    for( const tgCLight* pLight : rLightList )
    {
        rLightManager.SetCurrentLight( *pLight );

        for( tgUInt32 i = 0; i < m_pEnemyModel->GetNumMeshes(); ++i )
            CRenderCallBacks::MeshInstanceCB( *m_pEnemyModel->GetMesh( i ), m_ModelInstance.pInstanceBuffer, m_ModelInstance.NumMeshes );
    }

    if( !DoDebugRender )
        return;

    for( CEnemy* pEnemy : m_Enemies )
        pEnemy->Render();
}
