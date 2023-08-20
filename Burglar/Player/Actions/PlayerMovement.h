// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <InputAction.h>
#include "PlayerMovement.generated.h"

class AVRPlayer;

UCLASS( NotBlueprintable, NotPlaceable, NotBlueprintType )
class BURGLAR_API UPlayerMovement : public UObject
{
	GENERATED_BODY()

public:
	static void MoveForward( const FInputActionInstance& Instance, AVRPlayer* Player );
	static void MoveRight( const FInputActionInstance& Instance, AVRPlayer* Player );
	static void Teleport( const FInputActionInstance& Instance, AVRPlayer* Player );
	static void ToggleSprint( const FInputActionInstance& Instance, AVRPlayer* Player );
	static void SnapTurn( const FInputActionInstance& Instance, AVRPlayer* Player );
	static void MouseRotation( const FInputActionInstance& Instance, AVRPlayer* Player );
};
