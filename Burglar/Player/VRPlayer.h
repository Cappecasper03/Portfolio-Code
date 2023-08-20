// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Actions/PlayerMovement.h"
#include "Actions/PlayerHandInteractions.h"
#include "MotionSicknessHelper.h"

#include <InputAction.h>
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "VRPlayer.generated.h"

class UWidgetInteractionComponent;
class USphereComponent;
class UFloatingPawnMovement;
class UCapsuleComponent;
class USceneComponent;
class UCameraComponent;
class UInputMappingContext;
class UVRControllerComponent;
class UBoxComponent;
class UStealableComponent;

UCLASS( Blueprintable, BlueprintType, NotPlaceable )
class BURGLAR_API AVRPlayer : public APawn
{
	GENERATED_BODY()

	friend UPlayerMovement;
	friend UPlayerHandInteractions;

public:
	// Sets default values for this pawn's properties
	AVRPlayer();

	UFUNCTION( BlueprintImplementableEvent )
	void OnKillGreta();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward( const FInputActionInstance& Instance );
	void MoveRight( const FInputActionInstance& Instance );
	void Teleport( const FInputActionInstance& Instance );
	void ToggleSprint( const FInputActionInstance& Instance );
	void SnapTurn( const FInputActionInstance& Instance );
	void MouseRotation( const FInputActionInstance& Instance );

	void LeftGrabOrDrop( const FInputActionInstance& Instance );
	void LeftInteract( const FInputActionInstance& Instance );
	void RightGrabOrDrop( const FInputActionInstance& Instance );
	void RightInteract( const FInputActionInstance& Instance );

	void LeftGrasp( const FInputActionInstance& Instance );
	void RightGrasp( const FInputActionInstance& Instance );
	void LeftIndexCurl( const FInputActionInstance& Instance );
	void RightIndexCurl( const FInputActionInstance& Instance );

	UFUNCTION( BlueprintImplementableEvent )
	void OnWin();
	UFUNCTION( BlueprintImplementableEvent )
	void OnLose();
	UFUNCTION( BlueprintImplementableEvent )
	void OnFirstHitByGreta();

	UFUNCTION( BlueprintImplementableEvent, Category = "Sound" )
	void OnSteal( const AActor* StolenActor );
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound" )
	void OnNearStealable();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound" )
	void OnNearNeededStealable();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound" )
	void OnGrabItem( const AActor* GrabbedActor );
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound" )
	void OnDropItem( const AActor* DroppedActor );
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound" )
	void OnStepTaken( const TArray<FName>& FloorTags );
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound" )
	void OnStepOnActor( const AActor* ActorSteppedOn );

	UFUNCTION( BlueprintImplementableEvent, Category = "Grab" )
	void OnDistanceGrab( bool bIsRightController );

	void ToggleFPS( const FInputActionInstance& Instance );

	UFUNCTION()
	void OnMoneyBagBeginOverlap( UPrimitiveComponent*   OverlappedComponent
	                             , AActor*              OtherActor
	                             , UPrimitiveComponent* OtherComp
	                             , int32                OtherBodyIndex
	                             , bool                 bFromSweep
	                             , const FHitResult&    SweepResult );

	UFUNCTION()
	void OnNearBeginOverlap( UPrimitiveComponent*   OverlappedComponent
	                         , AActor*              OtherActor
	                         , UPrimitiveComponent* OtherComp
	                         , int32                OtherBodyIndex
	                         , bool                 bFromSweep
	                         , const FHitResult&    SweepResult );

	UFUNCTION()
	void OnFeetBeginOverlap( UPrimitiveComponent*   OverlappedComponent
	                         , AActor*              OtherActor
	                         , UPrimitiveComponent* OtherComp
	                         , int32                OtherBodyIndex
	                         , bool                 bFromSweep
	                         , const FHitResult&    SweepResult );

	UFUNCTION( BlueprintCallable )
	void ReportSound();

	UFUNCTION( BlueprintCallable )
	void TeleportToHitTeleportPoint();

	UFUNCTION( BlueprintCallable )
	FString DisplayMoneyTaken();

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
	UFloatingPawnMovement* FloatingPawnMovement;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Collider" )
	UCapsuleComponent* CapsuleCollider;

	UPROPERTY( EditDefaultsOnly, Category = "PickupRange" )
	float DistanceGrabRange;

	UPROPERTY()
	USceneComponent* VROrigin;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Camera" )
	UCameraComponent* Camera;

	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	TArray<UInputMappingContext*> InputMappingContexts;

	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* MoveForwardActionLeft;
	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* MoveRightActionLeft;
	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* MoveForwardActionRight;
	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* MoveRightActionRight;
	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* ToggleSprintAction;
	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* SnapTurnActionLeft;
	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* SnapTurnActionRight;
	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* MouseRotateAction;

	UPROPERTY( EditDefaultsOnly, Category = "Input" )
	UInputAction* ToggleFPSAction;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Controller" )
	UVRControllerComponent* LeftController;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Controller" )
	UVRControllerComponent* RightController;

	UPROPERTY( EditDefaultsOnly, Category = "Controller" )
	TArray<TEnumAsByte<EObjectTypeQuery>> GrabbableObjectTypes;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|SnapTurnAmount" )
	float SnapTurnAmount;

	UPROPERTY( BlueprintReadWrite, Category = "Settings|Movement" )
	bool bMoveWithLeftJoystick;
	UPROPERTY( BlueprintReadWrite, Category = "Settings|Movement" )
	bool bUseTeleportMovement;
	UPROPERTY()
	FVector MoveInputStrenght;
	UPROPERTY()
	float MoveSpeed;
	UPROPERTY( EditDefaultsOnly, Category = "MovementSpeed" )
	float WalkSpeed;
	UPROPERTY( EditDefaultsOnly, Category = "MovementSpeed" )
	float SprintSpeed;
	UPROPERTY()
	bool bIsSprinting;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina" )
	float MaxStamina;
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Stamina" )
	float CurrentStamina;
	UPROPERTY( EditDefaultsOnly, Category = "Stamina" )
	float StaminaGain;
	UPROPERTY( EditDefaultsOnly, Category = "Stamina" )
	float StaminaDrain;
	UPROPERTY()
	float StaminaTimer;
	UPROPERTY( EditDefaultsOnly, Category = "Stamina" )
	float StaminaUpdateInterval;
	UPROPERTY()
	float StaminaNeededToSprint;
	UPROPERTY( EditDefaultsOnly, Category = "Stamina" )
	float StaminaNeededWhenRecovering;

	UPROPERTY()
	USceneComponent* BagOrigin;
	UPROPERTY( EditDefaultsOnly, Category = "MoneyBag" )
	UStaticMeshComponent* BagMesh;
	UPROPERTY( EditDefaultsOnly, Category = "MoneyBag" )
	UBoxComponent* BagCollider;
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "MoneyBag" )
	TArray<UStealableComponent*> StolenObjects;

	UPROPERTY()
	bool bHasWon;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "MoneyBag" )
	int ValuablesToSteal;
	UPROPERTY( VisibleAnywhere, Category = "MoneyBag" )
	uint32 ObjectsNeededToFinish;
	UPROPERTY( EditDefaultsOnly, Category = "HapticFeedback" )
	USphereComponent* NearStealableSphere;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory" )
	TArray<UChildActorComponent*> InventoryItems;
	UPROPERTY()
	TArray<FRotator> InventoryItemRotations;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|HapticFeedback" )
	bool bUseHapticFeedback;
	UPROPERTY( EditDefaultsOnly, Category = "HapticFeedback" )
	UHapticFeedbackEffect_Base* NearNeededStealableFeedback;
	UPROPERTY( EditDefaultsOnly, Category = "HapticFeedback" )
	UHapticFeedbackEffect_Base* NearStealableFeedback;

	UPROPERTY( EditDefaultsOnly, Category = "HapticFeedback|PuzzleObjectClasses" )
	TArray<UClass*> PuzzleObjectClasses;
	UPROPERTY( EditDefaultsOnly, Category = "HapticFeedback" )
	UHapticFeedbackEffect_Base* NearPuzzleFeedback;

	UPROPERTY( EditDefaultsOnly, Category = "HapticFeedback" )
	UHapticFeedbackEffect_Base* HitHeadFeedback;
	UPROPERTY( EditDefaultsOnly, Category = "HapticFeedback" )
	UHapticFeedbackEffect_Base* HitHandFeedback;

	UPROPERTY( EditDefaultsOnly, Category = "AI" )
	UAIPerceptionStimuliSourceComponent* PerceptionStimSource;

	UPROPERTY( VisibleAnywhere, Category = "MotionSicknessHelper" )
	AMotionSicknessHelper* MotionSicknessHelper;

	UPROPERTY( EditDefaultsOnly, Category = "StepSound" )
	float TimeBetweenSteps;
	UPROPERTY()
	float StepTimer;
	UPROPERTY( EditDefaultsOnly, Category = "StepSound" )
	float DistanceBetweenSteps;
	UPROPERTY()
	FVector LastStepLocation;

	UPROPERTY( EditDefaultsOnly, Category = "Collider" )
	UBoxComponent* FeetCollider;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Settings|EyeTracking" )
	bool bUseEyeTrackingForMenus;
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "WidgetInteraction" )
	UWidgetInteractionComponent* EyeWidgetInteractor;

	UPROPERTY( EditDefaultsOnly, Category = "HealthPoints" )
	uint32 HealthPoints;
	UPROPERTY()
	AActor* HitTeleportPoint;
	UPROPERTY( EditDefaultsOnly, Category = "HitTeleportPoint" )
	UClass* HitTeleportBlueprintClass;

private:
	void UpdateMovement( float DeltaTime );
	void UpdateStamina( float DeltaTime );
	void UpdateCapsuleLocation();
	void UpdateDynamicHeight();
	void UpdateBagLocationAndRotation();
	void UpdateInventoryItemLocations();
	void UpdateStepSound( float DeltaTime, TArray<FName>& FloorTags );
	void UpdateMenuEyeTracking();

	bool bIsVR;
	bool bHasResetLocation;

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent( class UInputComponent* PlayerInputComponent ) override;

	bool IsSprinting() { return bIsSprinting; }

	void PlayHapticEffect( UHapticFeedbackEffect_Base* HapticEffect, EControllerHand Hand, float Scale = 1, bool bLoop = false );

	void Hit();
};
