//////////////////////////////////////////////////////////////////////////////////////////
//  File name: CPlayer.cpp                                                              //
//  Created:   2022-02-18 16:54:46                                                      //
//                                                                                      //
//                                                                                      //
//  Copyright (c) 2022 Tension Graphics AB                                              //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

#include	<tgSystem.h>

#include	"CPlayer.h"

#include	"Camera/CCamera.h"
#include	"Managers/CModelManager.h"
#include	"Managers/CWorldManager.h"
#include	"CApplication.h"
#include	"CClock.h"
#include	"CLevel.h"
#include	"Octree/COctree.h"
#include	"Octree/IOctreeObject.h"
#include	"Enemy/CEnemy.h"
#include	"GameStateMachine/CGameStates.h"

#include	<tgCDebugManager.h>
#include	<tgCAnimation.h>
#include	<tgCInterpolator.h>
#include	<tgCAnimationManager.h>
#include	<tgCCore.h>
#include	<tgCLine3D.h>
#include	<tgCCollision.h>
#include	<tgCLine2D.h>
#include	<tgLog.h>

////////////////////////////// CPlayer //////////////////////////////
//                                                                 //
//  Info:
//                                                                 //
//*//////////////////////////////////////////////////////////////////
CPlayer::CPlayer( void )
: m_pModel( NULL )
//, m_Animations
, m_pInterpolatorCurrent( NULL )
, m_pInterpolatorBlend( NULL )
, m_pInterpolatorNext( NULL )
, m_MovementDirection( tgCV3D::Zero )
, m_CameraRotation( 45.0f, 180.0f, 0.0f )
, m_Position( 3.0f, 0.0f, -1.0f )
, m_Rotation( 0.0f, 180.0f, 0.0f )
, m_Velocity( 0.0f, 0.0f, 0.0f )
, m_CameraMinAngle( -45.0f )
, m_CameraMaxAngle( 90.0f )
, m_WalkSpeed( 2.0f )
, m_RunSpeed( 5.0f )
, m_Blend( 0.0f )
, m_Radius( 0.5f )
, m_SlopeThreshold( 0.8f )
, m_AttackTime( 0.0f )
, m_HurtTime( 0.0f )
, m_State( STATE_IDLE )
, m_LastState( STATE_IDLE )
, m_IsControlling( true )
, m_IsBlending( false )
, m_Running( false )
, m_Grounded( false )
, m_KillCount( 0 )
, m_HasShot( false )
, m_SurvivalTime()
{
	// Load model
	m_pModel						= CModelManager::GetInstance().LoadModel( "models/orc", "Player", true );

	// Load animations
	m_Animations[ STATE_IDLE ]		= tgCAnimationManager::GetInstance().Create( "animations/orc_idle" );
	m_Animations[ STATE_WALK ]		= tgCAnimationManager::GetInstance().Create( "animations/orc_walk" );
	m_Animations[ STATE_RUN ]		= tgCAnimationManager::GetInstance().Create( "animations/orc_run" );
	m_Animations[ STATE_ATTACK_1 ]	= tgCAnimationManager::GetInstance().Create( "animations/orc_attack" );
	m_Animations[ STATE_HURT_1 ]	= tgCAnimationManager::GetInstance().Create( "animations/orc_hit" );
	m_Animations[ STATE_HURT_2 ]	= tgCAnimationManager::GetInstance().Create( "animations/orc_fall" );

	// Create interpolators
	m_pInterpolatorCurrent			= new tgCInterpolator( *m_Animations[ STATE_IDLE ] );
	m_pInterpolatorBlend			= new tgCInterpolator( *m_Animations[ STATE_IDLE ] );
	m_pInterpolatorNext				= new tgCInterpolator( *m_Animations[ STATE_IDLE ] );

	tgInput::AddListener( this );

}	// */ // CPlayer


////////////////////////////// ~CPlayer //////////////////////////////
//                                                                  //
//  Info:
//                                                                  //
//*///////////////////////////////////////////////////////////////////
CPlayer::~CPlayer( void )
{
	tgInput::RemoveListener( this );

	// Destroy interpolators
	delete m_pInterpolatorCurrent;
	delete m_pInterpolatorBlend;
	delete m_pInterpolatorNext;

	// Destroy animations
	for( tgCAnimation* pAnimation : m_Animations )
	{
		if( pAnimation )
			tgCAnimationManager::GetInstance().Destroy( &pAnimation );
	}

	// Destroy model
	if( m_pModel )
		CModelManager::GetInstance().DestroyModel( m_pModel );

}	// */ // ~CPlayer

////////////////////////////// Update //////////////////////////////
//                                                                //
//  Info:
//                                                                //
//*/////////////////////////////////////////////////////////////////
void
CPlayer::Update( const tgFloat DeltaTime )
{
	switch( m_State )
	{
		case STATE_IDLE:
		{
			if( m_MovementDirection.DotProduct() )
				m_State	= m_Running ? STATE_RUN : STATE_WALK;
		}
		break;

		case STATE_WALK:
		{
			if( !m_MovementDirection.DotProduct() )
				m_State	= STATE_IDLE;
			else if( m_Running )
				m_State	= STATE_RUN;
		}
		break;

		case STATE_RUN:
		{
			if( !m_MovementDirection.DotProduct() )
				m_State	= STATE_IDLE;
			else if( !m_Running )
				m_State	= STATE_WALK;
		}
		break;

		case STATE_ATTACK_1:
		{
			const tgFloat	AttackLength	 = 1.25f;

			m_AttackTime					+= DeltaTime;

			if( m_AttackTime >= AttackLength )
			{
				if( m_MovementDirection.DotProduct() )
					m_State	= m_Running ? STATE_RUN : STATE_WALK;
				else
					m_State	= STATE_IDLE;

				m_HasShot = false;	
			}
			else if(  m_AttackTime >= AttackLength * 0.8f && !m_HasShot )
			{
				Shoot();
				m_HasShot = true;	
			}
		}
		break;

		case STATE_HURT_1:
		case STATE_HURT_2:
		{
			const tgFloat	HurtLength	 = m_State == STATE_HURT_1 ? 0.6f : 1.0f;

			m_HurtTime					+= DeltaTime;

			if( m_HurtTime >= HurtLength )
			{
				if( m_MovementDirection.DotProduct() )
					m_State	= m_Running ? STATE_RUN : STATE_WALK;
				else
					m_State	= STATE_IDLE;
			}
		}
		break;
	}

//////////////////////////////////////////////////////////////////////////

	tgCCamera&	r3DCamera		= *CApplication::GetInstance().Get3DCamera()->GetCamera();
	tgCMatrix&	rCameraMatrix	= r3DCamera.GetTransform().GetMatrixLocal();

	if( CanMove() && m_MovementDirection.DotProduct() )
	{
		tgCV3D		MoveDir( -rCameraMatrix.Left.z, 0.0f, rCameraMatrix.Left.x );
		tgCMatrix	RotationMatrix	 = tgCMatrix::Identity;
		RotationMatrix.At			 = m_MovementDirection.Normalized();
		RotationMatrix.OrthoNormalize();
		MoveDir.TransformVector( RotationMatrix );

		tgFloat	SpeedModifier	= ( m_State == STATE_RUN ) ? m_RunSpeed : m_WalkSpeed;

		m_Position		+= MoveDir * SpeedModifier * DeltaTime;
	}
	
	m_Rotation.y = tgMathInterpolateLinear( m_Rotation.y, m_CameraRotation.y, DeltaTime * 15 );

//////////////////////////////////////////////////////////////////////////

	HandleCollision( DeltaTime );
	HandleAnimation( DeltaTime );

//////////////////////////////////////////////////////////////////////////

	m_CameraRotation.x	= tgMathClamp( m_CameraMinAngle, m_CameraRotation.x, m_CameraMaxAngle );

	rCameraMatrix.Translate( tgCV3D::NegativeZ * 5.0f, tgCMatrix::COMBINE_REPLACE );
	rCameraMatrix.RotateXYZ( m_CameraRotation, tgCMatrix::COMBINE_POST_MULTIPLY );
	rCameraMatrix.Translate( m_Position + tgCV3D::PositiveY * 1.8f - rCameraMatrix.Left * 0.8f, tgCMatrix::COMBINE_POST_MULTIPLY );

	HandleCameraCollision( rCameraMatrix );

	r3DCamera.GetTransform().Update();
	r3DCamera.CalcFrustum();

//////////////////////////////////////////////////////////////////////////

	m_pModel->GetTransform().GetMatrixLocal().Scale( 0.01f, tgCMatrix::COMBINE_REPLACE );
	m_pModel->GetTransform().GetMatrixLocal().RotateXYZ( m_Rotation, tgCMatrix::COMBINE_POST_MULTIPLY );
	m_pModel->GetTransform().GetMatrixLocal().Translate( m_Position, tgCMatrix::COMBINE_POST_MULTIPLY );
	m_pModel->GetTransform().Update();

	HandleEnemyCollision();

}	// */ // Update

void CPlayer::HandleCameraCollision( tgCMatrix& rCameraMatrix )
{
	const tgCLine3D Line( rCameraMatrix.Pos + rCameraMatrix.At * 5, rCameraMatrix.Pos );
	tgCCollision    Collision( true );
	Collision.SetType( tgCMesh::TYPE_WORLD );
	
	if( Collision.LineAllMeshesInWorld( Line, *CWorldManager::GetInstance().GetWorld( "Collision" ) ) )
		rCameraMatrix.Pos = Collision.GetLocalIntersection();
}

////////////////////////////// HandleCollision //////////////////////////////
//                                                                         //
//  Info:
//                                                                         //
//*//////////////////////////////////////////////////////////////////////////
void
CPlayer::HandleCollision( const tgFloat DeltaTime )
{
	const tgCWorld&	rWorld	 = *CWorldManager::GetInstance().GetWorld( "Collision" );

	m_Position				+= m_Velocity * DeltaTime;
	m_Grounded				 = CheckGrounded( rWorld );

	if( !m_Grounded )
	{
		const tgFloat	Gravity	 = 9.81f;

		m_Velocity.y			-= Gravity * DeltaTime;
	}

//////////////////////////////////////////////////////////////////////////

	const tgCSphere	Sphere( m_Position + tgCV3D( 0, m_Radius, 0 ), m_Radius );
	tgCCollision	Collision( true );
	Collision.SetType( tgCMesh::EType::TYPE_WORLD );

	if( Collision.SphereAllMeshesInWorld( Sphere, rWorld ) )
	{
		const tgCV3D	Normal	= Collision.GetLocalNormal();
		const tgFloat	Slope	= Normal.DotProduct( tgCV3D::PositiveY );

		if( Slope <= m_SlopeThreshold )
		{
			const tgFloat	InvFraction	 = ( 1.0f - Collision.GetFraction() );
			const tgCV3D	Push		 = ( Normal * InvFraction ) * Sphere.GetRadius();
			m_Position.x				+= Push.x;
			m_Position.z				+= Push.z;
		}
	}

}	// */ // HandleCollision


////////////////////////////// HandleAnimation //////////////////////////////
//                                                                         //
//  Info:
//                                                                         //
//*//////////////////////////////////////////////////////////////////////////
void
CPlayer::HandleAnimation( const tgFloat DeltaTime )
{
	if( m_LastState != m_State )
	{
		if( m_IsBlending )
		{
			m_Blend = m_pInterpolatorCurrent->GetTime();
			m_pInterpolatorCurrent->SetAnimation( *m_pInterpolatorNext->GetAnimation() );
			m_pInterpolatorCurrent->SetTime( m_pInterpolatorNext->GetTime() );
		}

		m_pInterpolatorNext->SetAnimation( *m_Animations[ m_State ] );
		m_IsBlending	= true;
		m_LastState		= m_State;
	}

	if( m_IsBlending )
	{
		m_pInterpolatorBlend->Blend( *m_pInterpolatorCurrent, *m_pInterpolatorNext, m_Blend, true );
		m_pModel->SetAnimationMatrices( *m_pInterpolatorBlend );
		m_Blend		+= DeltaTime * 3;

		if( m_Blend > 1 )
		{
			m_IsBlending	= false;
			m_Blend			= 0;
			m_pInterpolatorCurrent->SetAnimation( *m_pInterpolatorNext->GetAnimation() );
			m_pInterpolatorCurrent->SetTime( m_pInterpolatorNext->GetTime() );
		}
	}
	else
	{
		m_pModel->SetAnimationMatrices( *m_pInterpolatorCurrent );
	}

	m_pInterpolatorCurrent->AddTime( DeltaTime, true, true );
	m_pInterpolatorNext->AddTime( DeltaTime, true, true );

	m_pModel->Update();

}	// */ // HandleAnimation

void CPlayer::Render( void )
{
	const tgCCamera::SViewport& rViewport = CApplication::GetInstance().Get2DCamera()->GetCamera()->GetViewport();
	tgCDebugManager& rDebugManager = tgCDebugManager::GetInstance();
	tgCV2D ScreenCenter = tgCV2D( rViewport.Width * 0.5f, rViewport.Height * 0.5f );

	tgCV2D Line1 = tgCV2D( ScreenCenter.x - 5.0f, ScreenCenter.y );
	tgCV2D Line2 = tgCV2D( ScreenCenter.x + 5.0f, ScreenCenter.y );
	rDebugManager.AddLine2D( tgCLine2D( Line1, Line2 ), tgCColor::Red );

	Line1 = tgCV2D( ScreenCenter.x, ScreenCenter.y - 5.0f );
	Line2 = tgCV2D( ScreenCenter.x, ScreenCenter.y + 5.0f );
	rDebugManager.AddLine2D( tgCLine2D( Line1, Line2 ), tgCColor::Red );
}


////////////////////////////// InputEvent //////////////////////////////
//                                                                    //
//  Info:
//                                                                    //
//*/////////////////////////////////////////////////////////////////////
void
CPlayer::InputEvent( const tgInput::EType Type, const tgInput::SEvent* pEvent)
{

#if defined( TG_WINDOWS )

	switch( Type )
	{
		case tgInput::TYPE_MOUSE_MOVE_RELATIVE:
		{
			m_CameraRotation.y	-= ( pEvent->CurrPos.x / tgCCore::GetInstance().GetWindowWidth()  ) * 135.0f;
			m_CameraRotation.x	+= ( pEvent->CurrPos.y / tgCCore::GetInstance().GetWindowHeight() ) * 135.0f;
		}
		break;

		case tgInput::TYPE_MOUSE_SINGLE_CLICK:
		{
			if( pEvent->Mouse.ButtonId > 2 )
				break;

			if( CanAttack() )
			{
				m_State			= STATE_ATTACK_1;
				m_AttackTime	= 0.0f;
			}
		}
		break;

		case tgInput::TYPE_KEY_DOWN:
		{
			if( pEvent->Keyboard.Repeat != 0 )
				break;

			switch( pEvent->Keyboard.VKey )
			{
				case 'W':		{ m_MovementDirection.z	+= 1.0f;	} break;
				case 'S':		{ m_MovementDirection.z	-= 1.0f;	} break;
				case 'A':		{ m_MovementDirection.x	+= 1.0f;	} break;
				case 'D':		{ m_MovementDirection.x	-= 1.0f;	} break;
				case VK_SHIFT:	{ m_Running				 = true;	} break;

				case 'F':
				case 'G':
				{
					m_State		= pEvent->Keyboard.VKey == 'F' ? STATE_HURT_1 : STATE_HURT_2;
					m_HurtTime	= 0.0f;
				}
				break;
			}
		}
		break;

		case tgInput::TYPE_KEY_UP:
		{
			switch( pEvent->Keyboard.VKey )
			{
				case 'W':		{ m_MovementDirection.z	-= 1.0f;	} break;
				case 'S':		{ m_MovementDirection.z	+= 1.0f;	} break;
				case 'A':		{ m_MovementDirection.x	-= 1.0f;	} break;
				case 'D':		{ m_MovementDirection.x	+= 1.0f;	} break;
				case VK_SHIFT:	{ m_Running				 = false;	} break;
			}
		}
		break;
	}

#endif // TG_WINDOWS

}	// */ // InputEvent


////////////////////////////// CanMove //////////////////////////////
//                                                                 //
//  Info:
//                                                                 //
//*//////////////////////////////////////////////////////////////////
tgBool
CPlayer::CanMove( void ) const
{
	if( !m_IsControlling )
		return false;

	if( m_State == STATE_ATTACK_1 || m_State == STATE_HURT_1 || m_State == STATE_HURT_2 )
		return false;

	return true;

}	// */ // CanMove


////////////////////////////// CanAttack //////////////////////////////
//                                                                   //
//  Info:
//                                                                   //
//*////////////////////////////////////////////////////////////////////
tgBool
CPlayer::CanAttack( void ) const
{
	if( !CanMove() )
		return false;

	return true;

}	// */ // CanAttack


////////////////////////////// CheckGrounded //////////////////////////////
//                                                                       //
//  Info:
//                                                                       //
//*////////////////////////////////////////////////////////////////////////
tgBool
CPlayer::CheckGrounded( const tgCWorld& rCollisionWorld )
{
	if( m_Velocity.y > 0.0f )
		return false;

//////////////////////////////////////////////////////////////////////////

	tgCLine3D		Line( m_Position + tgCV3D( 0.0f, m_Radius, 0.0f ), m_Position + tgCV3D( 0.0f, -0.05f, 0.0f ) );
	tgCCollision	Collision( true );
	Collision.SetType( tgCMesh::EType::TYPE_WORLD );

	if( Collision.LineAllMeshesInWorld( Line, rCollisionWorld ) )
	{
		const tgFloat	Slope	= Collision.GetLocalNormal().DotProduct( tgCV3D::PositiveY );

		if( Slope > m_SlopeThreshold )
		{
			m_Position		= Collision.GetLocalIntersection();
			m_Velocity.y	= 0.0f;

			return true;
		}
	}

	return false;

} // */ // CheckGrounded

void CPlayer::HandleEnemyCollision( void )
{
	if( !m_IsControlling )
		return;

	const std::vector<SOctreeNode*> OctreeNodesWithObjects = CLevel::GetInstance().GetOctree()->GetNodesWithObjects();

	for( const SOctreeNode* pOctreeNode : OctreeNodesWithObjects )
	{
		if( !pOctreeNode->Box.PointInside( m_Position + tgCV3D::PositiveY ) )
			continue;

		for( IOctreeObject* pObject : pOctreeNode->Objects )
		{
			if( pObject->GetBoundingSphere() && pObject->GetBoundingSphere()->PointInside( m_Position + tgCV3D::PositiveY ) )
			{
				CGameStates::GetInstance().GetStateMenu()->SetScore( m_SurvivalTime.GetLifeTime(), m_KillCount );
				CGameStates::GetInstance().SetStateMenu();
			}
		}

		for( const SOctreeNode* pNeighbourNode : pOctreeNode->NeighbourNodes )
		{
			for( IOctreeObject* pObject : pNeighbourNode->Objects )
			{
				if( pObject->GetBoundingSphere() && pObject->GetBoundingSphere()->PointInside( m_Position + tgCV3D::PositiveY ) )
				{
					CGameStates::GetInstance().GetStateMenu()->SetScore( m_SurvivalTime.GetLifeTime(), m_KillCount );
					CGameStates::GetInstance().SetStateMenu();
				}
			}
		}

		return;
	}
}

void CPlayer::Shoot( void )
{
	const tgCCamera& r3DCamera     = *CApplication::GetInstance().Get3DCamera()->GetCamera();
	const tgCMatrix& rCameraMatrix = r3DCamera.GetTransform().GetMatrixLocal();

	const tgCLine3D                 ShotLine( rCameraMatrix.Pos, rCameraMatrix.Pos + rCameraMatrix.At * 25.0f );
	const std::vector<SOctreeNode*> OctreeNodesWithObjects = CLevel::GetInstance().GetOctree()->GetNodesWithObjects();

	for( const SOctreeNode* pOctreeNode : OctreeNodesWithObjects )
	{
		if( !ShotLine.Intersect( pOctreeNode->Box ) )
			continue;

		for( IOctreeObject* pObject : pOctreeNode->Objects )
		{
			if( !ShotLine.Intersect( *pObject->GetBoundingSphere() ) )
				continue;

			CEnemy* pEnemy = reinterpret_cast<CEnemy*>( pObject );
			if( !pEnemy )
				continue;

			pEnemy->SetDead();
			m_KillCount++;
		}
	}
}
