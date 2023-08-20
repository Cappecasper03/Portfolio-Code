// Fill out your copyright notice in the Description page of Project Settings.

#include "SnapInPlaceWhenGrabbedComponent.h"

#include <MotionControllerComponent.h>

// Sets default values for this component's properties
USnapInPlaceWhenGrabbedComponent::USnapInPlaceWhenGrabbedComponent()
	: LocationOffset()
	, RotationOffset()
	, bIsForRightController( false )
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void USnapInPlaceWhenGrabbedComponent::BeginPlay()
{
	Super::BeginPlay();

	USceneComponent* ThisAttachParent = GetAttachParent();
	if( !ThisAttachParent )
		return;

	LocationOffset = ThisAttachParent->GetRelativeLocation();
	RotationOffset = ThisAttachParent->GetRelativeRotation();
	ThisAttachParent->DestroyComponent( true );
}

void USnapInPlaceWhenGrabbedComponent::MoveForCorrectSnapGrab( UMotionControllerComponent* ControllerComponent )
{
	if( bIsForRightController )
	{
		if( ControllerComponent->MotionSource != "Right" )
			return;
	}
	else
	{
		if( ControllerComponent->MotionSource != "Left" )
			return;
	}

	FVector  HandLocation( 0 );
	FRotator HandRotation( 0 );

	TArray<USceneComponent*> ChildComponents;
	ControllerComponent->GetChildrenComponents( false, ChildComponents );
	for( USceneComponent* ChildComponent : ChildComponents )
	{
		// TODO: Change to skeletonmeshcomponent
		UStaticMeshComponent* SkeletalMeshComponent = Cast<UStaticMeshComponent>( ChildComponent );
		if( SkeletalMeshComponent )
		{
			HandLocation = SkeletalMeshComponent->GetComponentLocation();
			HandRotation = SkeletalMeshComponent->GetComponentRotation();
			break;
		}
	}

	AActor* Owner = GetOwner();

	Owner->SetActorRotation( HandRotation );
	Owner->SetActorLocation( HandLocation );

	Owner->AddActorLocalRotation( RotationOffset.GetInverse() );
	Owner->AddActorLocalOffset( -LocationOffset );
}
