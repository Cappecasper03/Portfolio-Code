// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHandInteractions.h"
#include "../VRControllerComponent.h"
#include "../VRPlayer.h"
#include "../../Interactables/GrabbableComponent.h"
#include "../../Interactables/InteractableComponent.h"

#include <Components/SphereComponent.h>
#include <Kismet/KismetSystemLibrary.h>
#include <Engine/OverlapInfo.h>
#include <PhysicsEngine/PhysicsHandleComponent.h>
#include <MotionControllerComponent.h>
#include <Components/WidgetInteractionComponent.h>

#include "Engine/SkeletalMeshSocket.h"

#define Debug if(GEngine) GEngine->AddOnScreenDebugMessage

void UPlayerHandInteractions::GrabOrDrop( const FInputActionInstance& Instance, const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller )
{
	if( !Player )
		return;

	if( Instance.GetValue().Get<float>() > .5f )
		Grab( bIsRightController, Player, Controller );
	else
		Drop( Controller, Player );
}

void UPlayerHandInteractions::Interact( const FInputActionInstance& Instance, UInteractableComponent* InteractableComponent )
{
	if( !InteractableComponent )
		return;

	InteractableComponent->Interact();
}

UGrabbableComponent* UPlayerHandInteractions::DistanceGrab( const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller )
{
	UGrabbableComponent* GrabbableComponent = GetDirectionalGrabbable( bIsRightController, Player, Controller );
	if( !GrabbableComponent )
		return nullptr;

	if( !GrabbableComponent->IsDistanceGrabbable() )
		return nullptr;

	// if we are holding the object in either hand
	if( GrabbableComponent == Player->LeftController->GrabbedComponent || GrabbableComponent == Player->RightController->GrabbedComponent )
	{
		Debug( -1, 5, FColor::Red, "Grabbable object was already held" );
		return nullptr;
	}

	GrabbableComponent->GetOwner()->SetActorLocation( Controller->GrabSphere->GetComponentLocation() );
	return GrabbableComponent;
}

void UPlayerHandInteractions::Grab( const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller )
{
	UGrabbableComponent* ClosestComponent = GetNearestGrabbable( bIsRightController, Player, Controller );

	if( !ClosestComponent )
	{
		ClosestComponent = DistanceGrab( bIsRightController, Player, Controller );
		Player->OnDistanceGrab( bIsRightController );

		if( !ClosestComponent )
			return;
	}

	Controller->GrabbedComponent = ClosestComponent;
	Player->OnGrabItem( Controller->GrabbedComponent->GetOwner() );
	ClosestComponent->SetGrabbingController( Controller->MotionController );
	ClosestComponent->OnPickup();
	Controller->WidgetInteractionComponent->SetActive( false );
	Controller->PhysicsHandle->GrabComponentAtLocationWithRotation( Cast<UPrimitiveComponent>( ClosestComponent->GetOwner()->GetRootComponent() )
	                                                                , ""
	                                                                , Controller->GrabSphere->GetComponentLocation()
	                                                                , Controller->GrabSphere->GetComponentRotation() );

	if( bIsRightController )
	{
		if( Controller->GrabbedComponent == Player->LeftController->GrabbedComponent )
		{
			Player->LeftController->GrabbedComponent = nullptr;
			Player->LeftController->PhysicsHandle->ReleaseComponent();
			Player->LeftController->WidgetInteractionComponent->SetActive( true );
		}
	}
	else
	{
		if( Controller->GrabbedComponent == Player->RightController->GrabbedComponent )
		{
			Player->RightController->GrabbedComponent = nullptr;
			Player->RightController->PhysicsHandle->ReleaseComponent();
			Player->RightController->WidgetInteractionComponent->SetActive( true );
		}
	}
}

UGrabbableComponent* UPlayerHandInteractions::GetNearestGrabbable( const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller )
{
	TArray<FHitResult>      OutHits;
	const TArray<AActor*>   ActorsToIgnore{ Player };
	const USphereComponent* CurrentSphere      = Controller->GrabSphere;
	const FVector           ControllerLocation = CurrentSphere->GetComponentLocation();
	const float             SphereRadius       = CurrentSphere->GetUnscaledSphereRadius();

	UKismetSystemLibrary::SphereTraceMultiForObjects( Player->GetWorld()
	                                                  , ControllerLocation
	                                                  , ControllerLocation
	                                                  , SphereRadius
	                                                  , Player->GrabbableObjectTypes
	                                                  , false
	                                                  , ActorsToIgnore
	                                                  , EDrawDebugTrace::None
	                                                  , OutHits
	                                                  , true );

	UGrabbableComponent* ClosestComponent = nullptr;
	float                ClosestDistance  = SphereRadius + 10;
	for( FHitResult& HitResult : OutHits )
	{
		const float CurrentDistance = ( HitResult.GetActor()->GetActorLocation() - ControllerLocation ).Length();

		if( CurrentDistance < ClosestDistance || !ClosestComponent )
		{
			UGrabbableComponent* CurrentComponent = Cast<UGrabbableComponent>( HitResult.GetActor()->GetComponentByClass( UGrabbableComponent::StaticClass() ) );

			if( CurrentComponent )
			{
				ClosestComponent = CurrentComponent;
				ClosestDistance  = CurrentDistance;
			}
		}
	}

	UGrabbableComponent* GrabbableComponentOnPlayer = GetGrabbableOnPlayer( bIsRightController, Player, Controller );
	if( GrabbableComponentOnPlayer )
		ClosestComponent = GrabbableComponentOnPlayer;

	return ClosestComponent;
}

UGrabbableComponent* UPlayerHandInteractions::GetGrabbableOnPlayer( const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller )
{
	UGrabbableComponent* GrabbableComponent = nullptr;

	if( bIsRightController && Player->LeftController->GrabbedComponent )
	{
		TArray<FOverlapInfo> Overlaps = Player->RightController->GrabSphere->GetOverlapInfos();

		for( FOverlapInfo& OverlapInfo : Overlaps )
		{
			USphereComponent* OverlappedComponent = Cast<USphereComponent>( OverlapInfo.OverlapInfo.Component );

			if( !OverlappedComponent )
				continue;

			if( OverlappedComponent == Player->LeftController->GrabSphere )
			{
				GrabbableComponent = Player->LeftController->GrabbedComponent;
				break;
			}
		}
	}
	else if( !bIsRightController && Player->RightController->GrabbedComponent )
	{
		TArray<FOverlapInfo> Overlaps = Player->LeftController->GrabSphere->GetOverlapInfos();

		for( FOverlapInfo& OverlapInfo : Overlaps )
		{
			USphereComponent* OverlappedComponent = Cast<USphereComponent>( OverlapInfo.OverlapInfo.Component );

			if( !OverlappedComponent )
				continue;

			if( OverlappedComponent == Player->RightController->GrabSphere )
			{
				GrabbableComponent = Player->RightController->GrabbedComponent;
				break;
			}
		}
	}

	return GrabbableComponent;
}

void UPlayerHandInteractions::Drop( UVRControllerComponent* Controller, AVRPlayer* Player )
{
	if( Controller->GrabbedComponent )
	{
		Player->OnDropItem( Controller->GrabbedComponent->GetOwner() );
		Controller->PhysicsHandle->ReleaseComponent();
		Controller->GrabbedComponent->OnDrop();
		if( !Player->bUseEyeTrackingForMenus )
			Controller->WidgetInteractionComponent->SetActive( true );
	}

	Controller->GrabbedComponent = nullptr;
}

UGrabbableComponent* UPlayerHandInteractions::GetDirectionalGrabbable( const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller )
{
	FHitResult            OutHit;
	const TArray<AActor*> ActorsToIgnore{ Player };
	const FVector         Location       = Controller->Mesh->GetSocketLocation( "DistanceGrab" );
	const FVector         TraceDirection = Controller->Mesh->GetSocketQuaternion( "DistanceGrab" ).GetForwardVector();

	const bool bHit = UKismetSystemLibrary::LineTraceSingleForObjects( Player->GetWorld()
	                                                                   , Location
	                                                                   , Location + TraceDirection * Player->DistanceGrabRange
	                                                                   , Player->GrabbableObjectTypes
	                                                                   , false
	                                                                   , ActorsToIgnore
	                                                                   , EDrawDebugTrace::ForDuration
	                                                                   , OutHit
	                                                                   , true );

	UGrabbableComponent* GrabbableComponent = nullptr;
	if( bHit )
		GrabbableComponent = dynamic_cast<UGrabbableComponent*>( OutHit.GetActor()->GetComponentByClass( UGrabbableComponent::StaticClass() ) );

	return GrabbableComponent;
}
