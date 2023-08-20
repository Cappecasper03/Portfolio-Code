// Fill out your copyright notice in the Description page of Project Settings.

#include "VRControllerComponent.h"
#include "VRPlayer.h"
#include "../Interactables/GrabbableComponent.h"
#include "Burglar/Interactables/InteractableComponent.h"
#include "HandAnimInstance.h"

#include <MotionControllerComponent.h>
#include <Components/SphereComponent.h>
#include <PhysicsEngine/PhysicsHandleComponent.h>
#include <Components/WidgetInteractionComponent.h>

// Sets default values for this component's properties
UVRControllerComponent::UVRControllerComponent()
	: GrabAction( nullptr )
	, InteractAction( nullptr )
	, GraspAction( nullptr )
	, IndexCurlAction( nullptr )
	, MotionController( nullptr )
	, GrabSphere( nullptr )
	, GrabbedComponent( nullptr )
	, PhysicsHandle( nullptr )
	, Mesh( nullptr )
	, WidgetInteractionComponent( nullptr )
	, bIsRightController( false )
	, AnimInstance( nullptr )
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UVRControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = Cast<UHandAnimInstance>( Mesh->GetAnimInstance() );
}

void UVRControllerComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if( !GrabbedComponent )
		return;

	GrabbedComponent->Update( PhysicsHandle, GrabSphere->GetComponentLocation(), GrabSphere->GetComponentRotation() );

	if( ( GrabbedComponent->GetOwner()->GetActorLocation() - MotionController->GetComponentLocation() ).Length() > GrabbedComponent->GetDropDistance())
		ClearGrabbedComponent();
}

void UVRControllerComponent::GrabOrDrop( const FInputActionInstance& Instance, AVRPlayer* Player )
{
	UPlayerHandInteractions::GrabOrDrop( Instance, bIsRightController, Player, this );
}

void UVRControllerComponent::Interact( const FInputActionInstance& Instance )
{
	UPlayerHandInteractions::Interact( Instance, Cast<UInteractableComponent>( GrabbedComponent ) );
}

void UVRControllerComponent::Grasp( const FInputActionInstance& Instance )
{
	if( !AnimInstance )
		return;

	if( Instance.GetTriggerEvent() == ETriggerEvent::Triggered )
		AnimInstance->SetGraspAlpha( Instance.GetValue().Get<float>() );
	else
		AnimInstance->SetGraspAlpha( 0 );
}

void UVRControllerComponent::IndexCurl( const FInputActionInstance& Instance )
{
	if( !AnimInstance )
		return;

	if( Instance.GetTriggerEvent() == ETriggerEvent::Triggered )
		AnimInstance->SetIndexCurlWeights( Instance.GetValue().Get<float>() );
	else
		AnimInstance->SetIndexCurlWeights( 0 );
}

void UVRControllerComponent::ClearGrabbedComponent()
{
	PhysicsHandle->ReleaseComponent();
	GrabbedComponent->OnDrop();
	GrabbedComponent = nullptr;
	WidgetInteractionComponent->SetActive( true );
}
