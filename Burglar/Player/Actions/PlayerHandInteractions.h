// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <InputAction.h>
#include "PlayerHandInteractions.generated.h"

class UGrabbableComponent;
class AVRPlayer;
class UVRControllerComponent;
class UInteractableComponent;

UCLASS( NotBlueprintable, NotPlaceable, NotBlueprintType )
class BURGLAR_API UPlayerHandInteractions : public UObject
{
	GENERATED_BODY()

public:
	static void GrabOrDrop( const FInputActionInstance& Instance, const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller );
	static void Interact( const FInputActionInstance& Instance, UInteractableComponent* InteractableComponent );

private:
	static void                 Grab( const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller );
	static UGrabbableComponent* GetNearestGrabbable( const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller );
	static UGrabbableComponent* GetGrabbableOnPlayer( const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller );
	static void                 Drop( UVRControllerComponent* Controller, AVRPlayer* Player );

	static UGrabbableComponent* DistanceGrab( const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller );
	static UGrabbableComponent* GetDirectionalGrabbable( const bool bIsRightController, AVRPlayer* Player, UVRControllerComponent* Controller );
};
