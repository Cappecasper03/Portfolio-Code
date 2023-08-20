// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "C_AnimInstance.generated.h"

/**
 *
 */
UCLASS()
class GAME_API UC_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UC_AnimInstance();

	virtual void NativeUpdateAnimation( float DeltaTimeX ) override;

	UFUNCTION()
		void ChangeAnimation( UAnimationAsset* pAnimation, bool ShouldLoop = true, float PlayRate = 1 );
	UFUNCTION()
		void SetAnimation( UAnimationAsset* pAnimation, bool ShouldLoop = true, float PlayRate = 1 );

	UFUNCTION()
		bool IsPlaying( UAnimationAsset* pAnimation = nullptr );
	UFUNCTION()
		float GetAnimTime( UAnimationAsset* pAnimation ); // Returns -1 if chosen animation isn't playing

protected:
	UPROPERTY( BlueprintReadOnly, Category = "Current Animations" )
		UAnimSequence* m_pCurrentAnimation;
	UPROPERTY( BlueprintReadOnly, Category = "Current Animations" )
		float m_CurrentAnimTime;
	UPROPERTY( BlueprintReadOnly, Category = "Current Animations" )
		bool m_IsLoopingCurrent;
	bool m_IsPlaying;

	UPROPERTY( BlueprintReadOnly, Category = "BlendingFrom Animations" )
		UAnimSequence* m_pBlendingFromAnimation;
	UPROPERTY( BlueprintReadOnly, Category = "BlendingFrom Animations" )
		float m_BlendingFromAnimTime;
	UPROPERTY( BlueprintReadOnly, Category = "BlendingFrom Animations" )
		bool m_IsLoopingBlendingFrom;

	UPROPERTY( BlueprintReadOnly, Category = "Animations" )
		float m_BlendFactor;

	float m_BlendTimer;
	float m_TimeToBlend; // Time it takes to blend

	float m_PlayRate;
};
