// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AnimInstance.h"

UC_AnimInstance::UC_AnimInstance()
	: m_pCurrentAnimation( nullptr )
	, m_CurrentAnimTime( 0 )
	, m_IsLoopingCurrent( true )
	, m_pBlendingFromAnimation( nullptr )
	, m_BlendingFromAnimTime( 0 )
	, m_IsLoopingBlendingFrom( true )
	, m_IsPlaying( true )
	, m_BlendFactor( 1 )
	, m_BlendTimer( 0 )
	, m_TimeToBlend( .1f )
	, m_PlayRate( 1 )
{
}

void UC_AnimInstance::NativeUpdateAnimation( float DeltaTimeX )
{
	Super::NativeUpdateAnimation( DeltaTimeX );

	if( !m_pCurrentAnimation )
		return;

	m_CurrentAnimTime += DeltaTimeX * m_PlayRate;

	if( m_CurrentAnimTime > m_pCurrentAnimation->GetPlayLength() )
	{
		m_CurrentAnimTime = 0;
		if( !m_IsLoopingCurrent )
		{
			m_IsPlaying = false;
			m_CurrentAnimTime = m_pCurrentAnimation->GetPlayLength();
		}
	}

	if( !m_pBlendingFromAnimation )
		return;

	m_BlendingFromAnimTime += DeltaTimeX * m_PlayRate;

	if( m_BlendingFromAnimTime > m_pBlendingFromAnimation->GetPlayLength() )
	{
		m_BlendingFromAnimTime = 0;
		if( !m_IsLoopingBlendingFrom )
			m_BlendingFromAnimTime = m_pBlendingFromAnimation->GetPlayLength();
	}

	m_BlendTimer += DeltaTimeX;
	m_BlendFactor = m_BlendTimer / m_TimeToBlend;

	if( m_BlendTimer > m_TimeToBlend )
	{
		m_BlendFactor = 1;
		m_BlendTimer = 0;
		m_pBlendingFromAnimation = nullptr;
	}
}

void UC_AnimInstance::ChangeAnimation( UAnimationAsset* pAnimation, bool ShouldLoop, float PlayRate )
{
	if( !pAnimation )
		return;

	if( m_pCurrentAnimation )
	{
		m_pBlendingFromAnimation = m_pCurrentAnimation;
		m_IsLoopingBlendingFrom = m_IsLoopingCurrent;
		m_BlendingFromAnimTime = m_CurrentAnimTime;
		m_BlendFactor = 0;
	}

	m_pCurrentAnimation = Cast<UAnimSequence>( pAnimation );
	m_IsLoopingCurrent = ShouldLoop;
	m_CurrentAnimTime = 0;
	m_IsPlaying = true;
	m_PlayRate = PlayRate;
}

void UC_AnimInstance::SetAnimation( UAnimationAsset* pAnimation, bool ShouldLoop, float PlayRate )
{
	if( !pAnimation )
		return;

	m_pCurrentAnimation = Cast<UAnimSequence>( pAnimation );
	m_IsLoopingCurrent = ShouldLoop;
	m_CurrentAnimTime = 0;
	m_IsPlaying = true;
	m_PlayRate = PlayRate;

	m_BlendFactor = 1;
	m_pBlendingFromAnimation = nullptr;
}

bool UC_AnimInstance::IsPlaying( UAnimationAsset* pAnimation )
{
	UAnimSequence* pAnimSequence = Cast<UAnimSequence>( pAnimation );

	if( !pAnimSequence )
		return m_IsPlaying;

	if( m_pCurrentAnimation )
	{
		if( pAnimSequence == m_pCurrentAnimation )
			return true;
	}

	return false;
}

float UC_AnimInstance::GetAnimTime( UAnimationAsset* pAnimation )
{
	UAnimSequence* pAnimSequence = Cast<UAnimSequence>( pAnimation );

	if( m_pCurrentAnimation == pAnimSequence )
		return m_CurrentAnimTime;

	return -1;
}