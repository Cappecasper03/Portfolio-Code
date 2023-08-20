#include <tgSystem.h>

#include "CEnemy.h"
#include "Octree/SOctreeNode.h"
#include "Specialization/CLevel.h"
#include "Navigation/CNavMesh.h"

#include <tgCCollision.h>
#include <tgCDebugManager.h>
#include <tgCLine2D.h>
#include <tgCLine3D.h>
#include <tgCMesh.h>
#include <tgCProfiling.h>
#include <tgMath.h>

CEnemy::CEnemy( const tgSize Id, const tgCV3D& Position, const tgCSphere& rBoundingSphere )
    : IOctreeObject( Id, &m_TransformMatrix.Pos, &m_CollisionSphere )
    , m_TransformMatrix( tgCMatrix::Identity )
    , m_BoundingSphere( rBoundingSphere )
    , m_CollisionSphere( rBoundingSphere.GetPos(), rBoundingSphere.GetRadius() * .8f )
    , m_SphereOffset( rBoundingSphere.GetPos() )
    , m_MovementSpeed( 2.5f )
    , m_RotationSpeed( 200 )
    , m_MaxDistanceToChangeTargetPoint( 1 )
    , m_TargetPoint( Position )
    , m_Path()
    , m_TimeToBeIdle( 1 )
    , m_IdleTimer( 0 )
    , m_IsIdle( false )
    , m_IsDead( false )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    m_TransformMatrix.RotateY( tgMathRandom( -180.f, 180.f ), tgCMatrix::COMBINE_PRE_MULTIPLY );
    m_TransformMatrix.Pos = Position;
    HandleGroundCollision();

    m_BoundingSphere.SetPos( m_TransformMatrix.Pos + m_SphereOffset );
    m_CollisionSphere.SetPos( m_TransformMatrix.Pos + m_SphereOffset );
}

void CEnemy::Update( const tgFloat DeltaTime, const tgBool ShouldUpdateTargetPoint )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    if( m_IsIdle )
        m_IdleTimer += DeltaTime;

    if( m_IdleTimer > m_TimeToBeIdle )
    {
        m_IdleTimer = 0;
        m_IsIdle    = false;
    }

    if( ShouldUpdateTargetPoint )
        UpdateTargetPoint();

    HandleCollisionAgainstOthers( DeltaTime );
    RotateTowardsTargetPoint( DeltaTime );

    if( !m_IsIdle )
    {
        const tgCV3D MovementDirection = m_TransformMatrix.At * tgCV3D( 1, 0, 1 );
        m_TransformMatrix.Pos += MovementDirection * m_MovementSpeed * DeltaTime;
    }

    HandleGroundCollision();
    HandleWallCollision( DeltaTime );

    m_BoundingSphere.SetPos( m_TransformMatrix.Pos + m_SphereOffset );
    m_CollisionSphere.SetPos( m_TransformMatrix.Pos + m_SphereOffset );
}

void CEnemy::UpdateTargetPoint( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    if( m_Path.size() > 1 )
    {
        tgUInt32 ClosestIndex    = 0;
        tgFloat  ClosestDistance = TG_FLOAT_MAX;
        for( tgUInt32 i = 0; i < m_Path.size(); ++i )
        {
            const tgFloat CurrentDistance = ( *m_Path[i] - m_TransformMatrix.Pos ).Length();

            if( CurrentDistance < ClosestDistance )
            {
                ClosestDistance = CurrentDistance;
                ClosestIndex    = i;
            }
        }

        const std::vector<tgCLine3D>& rNavMeshEdges = CLevel::GetInstance().GetNavMesh()->GetEdges();
        tgCLine2D                     ThisToPathPoint( tgCV2D( m_TransformMatrix.Pos.x, m_TransformMatrix.Pos.z ), 0 );
        tgUInt32                      FurthestSeeingIndex = 0;

        for( tgSInt32 i = m_Path.size() - 1; i >= 0; --i )
        {
            const tgCV3D* pPathPoint = m_Path[i];
            ThisToPathPoint.SetEnd( tgCV2D( pPathPoint->x, pPathPoint->z ) );

            tgBool LinesIntersected = false;
            for( const tgCLine3D& rNavMeshEdge : rNavMeshEdges )
            {
                const tgCV3D&   rStartPoint = rNavMeshEdge.GetStart();
                const tgCV3D&   rEndPoint   = rNavMeshEdge.GetEnd();
                const tgCLine2D EdgeLine( tgCV2D( rStartPoint.x, rStartPoint.z ), tgCV2D( rEndPoint.x, rEndPoint.z ) );

                if( EdgeLine.Intersect( ThisToPathPoint ) )
                {
                    LinesIntersected = true;
                    break;
                }
            }

            if( !LinesIntersected )
            {
                FurthestSeeingIndex = i;
                break;
            }
        }

        if( FurthestSeeingIndex == m_Path.size() - 1 )
        {
            m_TargetPoint = *m_Path.back();
            return;
        }

        if( ClosestIndex < FurthestSeeingIndex )
            m_TargetPoint = *m_Path[FurthestSeeingIndex];
        else
            m_TargetPoint = *m_Path[FurthestSeeingIndex + 1];
    }
}

void CEnemy::RotateTowardsTargetPoint( const tgFloat DeltaTime )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const tgCV3D ToTargetPoint = m_TransformMatrix.Pos - m_TargetPoint;
    tgCV3D       CrossProduct( 0 );

    CrossProduct.CrossProduct( ToTargetPoint, m_TransformMatrix.At );
    if( CrossProduct.y < 0 )
        m_TransformMatrix.RotateY( -m_RotationSpeed * DeltaTime, tgCMatrix::COMBINE_PRE_MULTIPLY );
    else
        m_TransformMatrix.RotateY( m_RotationSpeed * DeltaTime, tgCMatrix::COMBINE_PRE_MULTIPLY );
}

void CEnemy::HandleGroundCollision( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const tgCLine3D Line( m_TransformMatrix.Pos + tgCV3D( 0, 1, 0 ), m_TransformMatrix.Pos - tgCV3D( 0, 3, 0 ) );
    tgCCollision    Collision( true );

    Collision.SetType( tgCMesh::TYPE_WORLD );
    if( Collision.LineAllMeshesInWorld( Line, *CLevel::GetInstance().GetCollisionWorld() ) )
        m_TransformMatrix.Pos.y = Collision.GetLocalIntersection().y + .3f;
}

void CEnemy::HandleWallCollision( const tgFloat DeltaTime )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgCCollision Collision( true );
    Collision.SetType( tgCMesh::TYPE_WORLD );

    if( Collision.SphereAllMeshesInWorld( m_CollisionSphere, *CLevel::GetInstance().GetCollisionWorld() ) )
    {
        m_TransformMatrix.Pos += Collision.GetLocalNormal() * ( m_CollisionSphere.GetRadius() * ( 1 - Collision.GetFraction() ) );

        if( !m_IsIdle )
        {
            tgCV3D CrossProduct( 0 );
            CrossProduct.CrossProduct( -Collision.GetLocalNormal(), m_TransformMatrix.At );
            if( CrossProduct.y < 0 )
                m_TransformMatrix.RotateY( -m_RotationSpeed * DeltaTime * 3, tgCMatrix::COMBINE_PRE_MULTIPLY );
            else
                m_TransformMatrix.RotateY( m_RotationSpeed * DeltaTime * 3, tgCMatrix::COMBINE_PRE_MULTIPLY );
        }
    }
}

void CEnemy::HandleCollisionAgainstOthers( const tgFloat DeltaTime )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    if( !m_pCurrentNode )
        return;

    tgUInt32 NumberOfTurnAways = 0;
    for( IOctreeObject* pOctreeObject : m_pCurrentNode->Objects )
    {
        const CEnemy* pOtherEnemy = reinterpret_cast<CEnemy*>( pOctreeObject );
        if( pOtherEnemy == this )
            continue;

        if( !m_IsIdle )
        {
            const tgCSphere& rOtherBoundingSphere = pOtherEnemy->m_BoundingSphere;

            if( rOtherBoundingSphere.Intersect( m_BoundingSphere ) )
            {
                TurnAwayFromOther( pOtherEnemy, DeltaTime );
                NumberOfTurnAways++;
            }
        }

        CollideWithOther( pOtherEnemy );
        m_CollisionSphere.SetPos( m_TransformMatrix.Pos + m_SphereOffset );
    }

    for( const SOctreeNode* pNode : m_CurrentNeighbourNodes )
    {
        for( IOctreeObject* pOctreeObject : pNode->Objects )
        {
            const CEnemy* pOtherEnemy = reinterpret_cast<CEnemy*>( pOctreeObject );
            if( pOtherEnemy == this )
                continue;

            if( !m_IsIdle )
            {
                const tgCSphere& rOtherBoundingSphere = pOtherEnemy->m_BoundingSphere;

                if( rOtherBoundingSphere.Intersect( m_BoundingSphere ) )
                {
                    TurnAwayFromOther( pOtherEnemy, DeltaTime );
                    NumberOfTurnAways++;
                }
            }

            CollideWithOther( pOtherEnemy );
            m_CollisionSphere.SetPos( m_TransformMatrix.Pos + m_SphereOffset );
        }
    }

    if( NumberOfTurnAways >= 7 )
    {
        m_IdleTimer = 0;
        m_IsIdle    = true;
    }
}

void CEnemy::Render( void )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    tgCDebugManager& rDebugManager = tgCDebugManager::GetInstance();

    rDebugManager.AddLine3D( tgCLine3D( m_TransformMatrix.Pos, m_TargetPoint ), tgCColor::Blue );

    if( m_pCurrentNode )
        rDebugManager.AddLine3D( tgCLine3D( m_pCurrentNode->Center, m_TransformMatrix.Pos ), tgCColor::Red );

    for( const SOctreeNode* pNode : m_CurrentNeighbourNodes )
        rDebugManager.AddLine3D( tgCLine3D( pNode->Center, m_TransformMatrix.Pos ), tgCColor::Red );
}

void CEnemy::TurnAwayFromOther( const CEnemy* pOther, const tgFloat DeltaTime )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const tgCSphere& rOtherBoundingSphere = pOther->m_BoundingSphere;
    const tgCV3D     ThisToOther          = rOtherBoundingSphere.GetPos() - m_BoundingSphere.GetPos();
    tgCV3D           CrossProduct( 0 );

    CrossProduct.CrossProduct( ThisToOther, m_TransformMatrix.At );
    if( CrossProduct.y < 0 )
        m_TransformMatrix.RotateY( -m_RotationSpeed * DeltaTime * 2, tgCMatrix::COMBINE_PRE_MULTIPLY );
    else
        m_TransformMatrix.RotateY( m_RotationSpeed * DeltaTime * 2, tgCMatrix::COMBINE_PRE_MULTIPLY );
}

void CEnemy::CollideWithOther( const CEnemy* pOther )
{
#if !defined( FINAL )
    tgProfilingScope( __TG_FUNC__ );
#endif // !FINAL

    const tgCSphere& rOtherCollisionSphere = pOther->m_CollisionSphere;
    if( m_CollisionSphere.Intersect( rOtherCollisionSphere ) )
    {
        const tgCV3D  OtherToThis        = m_CollisionSphere.GetPos() - rOtherCollisionSphere.GetPos();
        const tgFloat IntersectionLength = ( m_CollisionSphere.GetRadius() * 2 ) - OtherToThis.Length();

        m_TransformMatrix.Pos += OtherToThis.Normalized() * IntersectionLength;
    }
}
