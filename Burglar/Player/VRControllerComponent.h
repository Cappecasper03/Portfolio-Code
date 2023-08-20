// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Actions/PlayerHandInteractions.h"
#include "Actions/PlayerMovement.h"

#include <InputAction.h>
#include "VRControllerComponent.generated.h"

class UWidgetInteractionComponent;
class UMotionControllerComponent;
class UGrabbableComponent;
class USphereComponent;
class USkeletalMesh;
class UPhysicsHandleComponent;
class UHandAnimInstance;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BURGLAR_API UVRControllerComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

	friend UPlayerHandInteractions;
	friend UPlayerMovement;

public:
	// Sets default values for this component's properties
	UVRControllerComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* GrabAction;
	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* InteractAction;
	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* GraspAction;
	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* IndexCurlAction;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Controller" )
	UMotionControllerComponent* MotionController;

	UPROPERTY( EditDefaultsOnly, Category = "Controller" )
	USphereComponent* GrabSphere;
	UPROPERTY( VisibleAnywhere, Category = "Controller" )
	UGrabbableComponent* GrabbedComponent;
	UPROPERTY( EditDefaultsOnly, Category = "Controller" )
	UPhysicsHandleComponent* PhysicsHandle;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Controller" )
	USkeletalMeshComponent* Mesh;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "WidgetInteraction" )
	UWidgetInteractionComponent* WidgetInteractionComponent;

	UPROPERTY( EditDefaultsOnly, Category = "Controller" )
	bool bIsRightController;

	UPROPERTY()
	UHandAnimInstance* AnimInstance;

public:
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UInputAction* GetGrabAction() { return GrabAction; }
	UInputAction* GetInteractAction() { return InteractAction; }
	UInputAction* GetGraspAction() { return GraspAction; }
	UInputAction* GetIndexCurlAction() { return IndexCurlAction; }

	void GrabOrDrop( const FInputActionInstance& Instance, AVRPlayer* Player );
	void Interact( const FInputActionInstance& Instance );

	void Grasp( const FInputActionInstance& Instance );
	void Point( const FInputActionInstance& Instance );
	void IndexCurl( const FInputActionInstance& Instance );
	void ThumbUp( const FInputActionInstance& Instance );

	void DistanceGrab( const FInputActionInstance& Instance, AVRPlayer* Player );


	void SetIsRightController( const bool bNewIsRightController ) { bIsRightController = bNewIsRightController; }
	void SetController( UMotionControllerComponent* NewController ) { MotionController = NewController; }
	void SetGrabSphere( USphereComponent* NewGrabSphere ) { GrabSphere = NewGrabSphere; }
	void SetPhysicsHandle( UPhysicsHandleComponent* NewPhysicsHandle ) { PhysicsHandle = NewPhysicsHandle; }
	void SetHandMesh( USkeletalMeshComponent* NewMesh ) { Mesh = NewMesh; }
	void SetWidgetInteractionComponent( UWidgetInteractionComponent* NewWidgetInteractionComponent ) { WidgetInteractionComponent = NewWidgetInteractionComponent; }

	UFUNCTION( BlueprintCallable )
	void ClearGrabbedComponent();

	UGrabbableComponent*        GetGrabbedComponent() { return GrabbedComponent; }
	UMotionControllerComponent* GetMotionController() { return MotionController; }
};
