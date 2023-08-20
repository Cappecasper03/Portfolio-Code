#pragma once

#include "Octree/IOctreeObject.h"

#include <tgCMatrix.h>
#include <tgCSphere.h>

#include <tgMemoryDisable.h>
#include <vector>
#include <tgMemoryEnable.h>

class CEnemy : public IOctreeObject
{
public:
    CEnemy( const tgSize Id, const tgCV3D& Position, const tgCSphere& rBoundingSphere );

    void Update( const tgFloat DeltaTime, const tgBool ShouldUpdateTargetPoint = true );
    void UpdateTargetPoint( void );
    void RotateTowardsTargetPoint( const tgFloat DeltaTime );
    void HandleGroundCollision( void );
    void HandleWallCollision( const tgFloat DeltaTime );
    void HandleCollisionAgainstOthers( const tgFloat DeltaTime );

    void Render( void );

    const tgCMatrix& GetTransformMatrix( void ) { return m_TransformMatrix; }
    void             SetPosition( const tgCV3D& NewPosition ) { m_TransformMatrix.Pos = NewPosition; }

    const tgCSphere& GetBoundingSphere( void ) { return m_BoundingSphere; }
    const tgCSphere& GetBoundingSphere( void ) const { return m_BoundingSphere; }
    const tgCSphere& GetCollisionSphere( void ) { return m_CollisionSphere; }
    const tgCSphere& GetCollisionSphere( void ) const { return m_CollisionSphere; }

    void SetPath( const std::vector<const tgCV3D*>& rPath ) { m_Path = rPath; }
    void SetTargetPoint( const tgCV3D& rTargetPoint ) { m_TargetPoint = rTargetPoint; }

    tgBool IsDead( void ) { return m_IsDead; }
    void   SetDead( const tgBool NewDeadState = true ) { m_IsDead = NewDeadState; }

protected:
    tgCMatrix m_TransformMatrix;
    tgCSphere m_BoundingSphere;
    tgCSphere m_CollisionSphere;
    tgCV3D    m_SphereOffset;

    tgFloat m_MovementSpeed;
    tgFloat m_RotationSpeed;

    tgFloat                    m_MaxDistanceToChangeTargetPoint;
    tgCV3D                     m_TargetPoint;
    std::vector<const tgCV3D*> m_Path;

    tgFloat m_TimeToBeIdle;
    tgFloat m_IdleTimer;
    tgBool  m_IsIdle;

    tgBool m_IsDead;

private:
    void TurnAwayFromOther( const CEnemy* pOther, const tgFloat DeltaTime );
    void CollideWithOther( const CEnemy* pOther );
};
