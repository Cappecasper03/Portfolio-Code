// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <PhysicsEngine/PhysicsHandleComponent.h>
#include "GrabbableComponent.generated.h"

class UMotionControllerComponent;
class UPhysicsHandleComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BURGLAR_API UGrabbableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGrabbableComponent();

protected:
	virtual void BeginPlay() override;

	void DelayedHandCollisionActivation();

	UPROPERTY( EditAnywhere, Category = "Physics" )
	bool bShouldSimulatePhysicsWhenGrabbed;
	UPROPERTY( EditAnywhere, Category = "Physics" )
	bool bShouldSimulatePhysicsWhenDropped;

	UPROPERTY( EditAnywhere, Category = "Collision" )
	bool bShouldHaveCollisionWhenGrabbed;
	UPROPERTY( EditAnywhere, Category = "Collision" )
	bool bShouldHaveCollisionWhenDropped;

	UPROPERTY( EditAnywhere, Category = "Gravity" )
	bool bShouldHaveGravityWhenGrabbed;
	UPROPERTY( EditAnywhere, Category = "Gravity" )
	bool bShouldHaveGravityWhenDropped;

	UPROPERTY( EditAnywhere, Category = "Grabbing" )
	bool bUseDistanceGrab;

	UPROPERTY( BlueprintReadWrite )
	bool bIsGrabbed;

	UPROPERTY( BlueprintReadWrite, meta = (AllowPrivateAccess = "true") )
	UMotionControllerComponent* GrabbingController;

	UPROPERTY(EditDefaultsOnly, Category = "Grabbing")
	float DropDistance;

	UPROPERTY()
	FTimerHandle TimerHandle;

public:
	virtual void OnPickup();

	virtual void OnDrop();

	virtual void Update( UPhysicsHandleComponent* PhysicsHandle, const FVector TargetLocation, const FRotator TargetRotation );

	void SetGrabbingController( UMotionControllerComponent* Controller );

	bool IsDistanceGrabbable() { return bUseDistanceGrab; }
	
	float GetDropDistance() { return DropDistance; }
};
