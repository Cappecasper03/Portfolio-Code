// Fill out your copyright notice in the Description page of Project Settings.

#include "GrabbableComponent.h"
#include "../Player/VRControllerComponent.h"
#include "SnapInPlaceWhenGrabbedComponent.h"

#include <MotionControllerComponent.h>
#include <PhysicsEngine/PhysicsHandleComponent.h>

#include "SnapInPlaceWhenGrabbedComponent.h"

// Sets default values for this component's properties
UGrabbableComponent::UGrabbableComponent()
	: bShouldSimulatePhysicsWhenGrabbed( true )
	, bShouldSimulatePhysicsWhenDropped( true )
	, bShouldHaveCollisionWhenGrabbed( true )
	, bShouldHaveCollisionWhenDropped( true )
	, bShouldHaveGravityWhenGrabbed( false )
	, bShouldHaveGravityWhenDropped( true )
	, bUseDistanceGrab( true )
	, bIsGrabbed( false )
	, DropDistance( 50.0f )
	, TimerHandle()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UGrabbableComponent::BeginPlay()
{
	Super::BeginPlay();

	UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>( GetOwner()->GetRootComponent() );
	if( !RootComponent )
		return;

	RootComponent->SetCollisionProfileName( "Grabbable" );
	RootComponent->SetGenerateOverlapEvents( true );

	RootComponent->SetEnableGravity( bShouldHaveGravityWhenDropped );
	RootComponent->SetSimulatePhysics( bShouldSimulatePhysicsWhenDropped );

	if( bShouldHaveCollisionWhenDropped )
	{
		RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel2, ECR_Block );
		RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel3, ECR_Block );
	}
	else
	{
		RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel2, ECR_Overlap );
		RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel3, ECR_Overlap );
	}
}

void UGrabbableComponent::DelayedHandCollisionActivation()
{
	const AActor*        Owner         = GetOwner();
	UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>( Owner->GetRootComponent() );
	if( !RootComponent )
		return;

	if( bShouldHaveCollisionWhenDropped )
		RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel3, ECR_Block );
}

void UGrabbableComponent::OnPickup()
{
	bIsGrabbed = true;

	const AActor* Owner = GetOwner();

	TArray<USnapInPlaceWhenGrabbedComponent*> SnapGrabComponents;
	Owner->GetComponents( SnapGrabComponents );
	for( USnapInPlaceWhenGrabbedComponent* SnapGrabComponent : SnapGrabComponents )
		SnapGrabComponent->MoveForCorrectSnapGrab( GrabbingController );

	UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>( Owner->GetRootComponent() );
	if( !RootComponent )
		return;

	RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel3, ECR_Overlap );
	RootComponent->SetEnableGravity( bShouldHaveGravityWhenGrabbed );
	RootComponent->SetSimulatePhysics( bShouldSimulatePhysicsWhenGrabbed );

	if( bShouldHaveCollisionWhenGrabbed )
		RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel2, ECR_Block );
	else
		RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel2, ECR_Overlap );
}

void UGrabbableComponent::OnDrop()
{
	bIsGrabbed = false;

	const AActor*        Owner         = GetOwner();
	UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>( Owner->GetRootComponent() );
	if( !RootComponent )
		return;

	RootComponent->SetEnableGravity( bShouldHaveGravityWhenDropped );
	RootComponent->SetSimulatePhysics( bShouldSimulatePhysicsWhenDropped );

	if( bShouldHaveCollisionWhenDropped )
	{
		RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel2, ECR_Block );
		GetOwner()->GetWorldTimerManager().SetTimer( TimerHandle, this, &UGrabbableComponent::DelayedHandCollisionActivation, .3f, false );
	}
	else
	{
		RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel2, ECR_Overlap );
		RootComponent->SetCollisionResponseToChannel( ECC_GameTraceChannel3, ECR_Overlap );
	}

	GrabbingController = nullptr;
}

void UGrabbableComponent::Update( UPhysicsHandleComponent* PhysicsHandle, const FVector TargetLocation, const FRotator TargetRotation )
{
	PhysicsHandle->SetTargetLocationAndRotation( TargetLocation, TargetRotation );
}

void UGrabbableComponent::SetGrabbingController( UMotionControllerComponent* Controller )
{
	GrabbingController = Controller;
}
