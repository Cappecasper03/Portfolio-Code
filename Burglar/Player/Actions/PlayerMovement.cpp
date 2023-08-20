// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerMovement.h"
#include "../VRPlayer.h"

#include <Camera/CameraComponent.h>
#include <Burglar/Player/VRControllerComponent.h>
#include <Kismet/KismetSystemLibrary.h>

void UPlayerMovement::MoveForward( const FInputActionInstance& Instance, AVRPlayer* Player )
{
	if( !Player )
		return;

	const FString ActionDesc = Instance.GetSourceAction()->ActionDescription.ToString();

	if( ( ActionDesc == "Left" && Player->bMoveWithLeftJoystick ) || ( ActionDesc == "Right" && !Player->bMoveWithLeftJoystick ) )
		Player->MoveInputStrenght.X = Instance.GetValue().Get<float>();
}

void UPlayerMovement::MoveRight( const FInputActionInstance& Instance, AVRPlayer* Player )
{
	if( !Player )
		return;

	const FString ActionDesc = Instance.GetSourceAction()->ActionDescription.ToString();

	if( ( ActionDesc == "Left" && Player->bMoveWithLeftJoystick ) || ( ActionDesc == "Right" && !Player->bMoveWithLeftJoystick ) )
		Player->MoveInputStrenght.Y = Instance.GetValue().Get<float>();
}

void UPlayerMovement::Teleport( const FInputActionInstance& Instance, AVRPlayer* Player )
{
	if( !Player )
		return;
	else if( !Player->bUseTeleportMovement )
		return;

	const FString ActionDesc = Instance.GetSourceAction()->ActionDescription.ToString();

	if( ( ActionDesc == "Left" && Player->bMoveWithLeftJoystick ) || ( ActionDesc == "Right" && !Player->bMoveWithLeftJoystick ) )
	{
		FVector Offset( 0 );
		Offset += Player->Camera->GetForwardVector() * 50;
		Offset.Z = 0;

		Player->AddActorWorldOffset( Offset );
	}
}

void UPlayerMovement::ToggleSprint( const FInputActionInstance& Instance, AVRPlayer* Player )
{
	if( !Player )
		return;

	Player->bIsSprinting = Instance.GetValue().Get<bool>();

	if( Player->CurrentStamina < Player->StaminaNeededToSprint )
		Player->bIsSprinting = false;

	if( Player->bIsSprinting )
		Player->MoveSpeed = Player->SprintSpeed;
	else
		Player->MoveSpeed = Player->WalkSpeed;
}

void UPlayerMovement::SnapTurn( const FInputActionInstance& Instance, AVRPlayer* Player )
{
	if( !Player )
		return;

	const FString ActionDesc = Instance.GetSourceAction()->ActionDescription.ToString();

	if( ( ActionDesc == "Left" && Player->bMoveWithLeftJoystick ) || ( ActionDesc == "Right" && !Player->bMoveWithLeftJoystick ) )
		return;

	const FVector OldCameraLocation = Player->Camera->GetComponentLocation();

	if( Instance.GetValue().Get<float>() > 0 )
		Player->VROrigin->AddRelativeRotation( FRotator( 0, Player->SnapTurnAmount, 0 ) );
	else
		Player->VROrigin->AddRelativeRotation( FRotator( 0, -Player->SnapTurnAmount, 0 ) );

	FVector NewToOldCameraLocation = OldCameraLocation - Player->Camera->GetComponentLocation();
	NewToOldCameraLocation.Z       = 0;

	Player->VROrigin->AddWorldOffset( NewToOldCameraLocation );
}

void UPlayerMovement::MouseRotation( const FInputActionInstance& Instance, AVRPlayer* Player )
{
	if( !Player )
		return;

	const FVector2D ActionVector = Instance.GetValue().Get<FVector2D>();

	Player->Camera->AddRelativeRotation( FRotator( ActionVector.X, ActionVector.Y, 0 ) * 80 );
}
