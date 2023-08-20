// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HandAnimInstance.generated.h"

UCLASS()
class BURGLAR_API UHandAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UHandAnimInstance();

protected:
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Pose" )
	UAnimSequenceBase* Idle;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Pose" )
	UAnimSequenceBase* Grasp;
	UPROPERTY( BlueprintReadOnly, Category = "Pose" )
	float GraspAlpha;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Pose" )
	UAnimSequenceBase* IndexCurl;
	UPROPERTY( BlueprintReadOnly, Category = "Pose" )
	float IndexCurlWeights;

public:
	void SetGraspAlpha( float NewAlpha ) { GraspAlpha = NewAlpha; }
	void SetIndexCurlWeights( float NewWeights ) { IndexCurlWeights = NewWeights; }
};
