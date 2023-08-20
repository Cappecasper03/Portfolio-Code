// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "SnapInPlaceWhenGrabbedComponent.generated.h"

class UMotionControllerComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BURGLAR_API USnapInPlaceWhenGrabbedComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USnapInPlaceWhenGrabbedComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	FVector LocationOffset;

	UPROPERTY()
	FRotator RotationOffset;

	UPROPERTY( EditAnywhere, Category = "Controller" )
	bool bIsForRightController;

public:
	void MoveForCorrectSnapGrab( UMotionControllerComponent* ControllerComponent );
};
