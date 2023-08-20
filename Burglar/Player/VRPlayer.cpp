// Fill out your copyright notice in the Description page of Project Settings.

#include "VRPlayer.h"
#include "VRControllerComponent.h"
#include "../Interactables/StealableComponent.h"

#include <GameFramework/FloatingPawnMovement.h>
#include <Components/CapsuleComponent.h>
#include <Camera/CameraComponent.h>
#include <InputMappingContext.h>
#include <HeadMountedDisplayFunctionLibrary.h>
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>
#include <MotionControllerComponent.h>
#include <Components/SphereComponent.h>
#include <Kismet/KismetSystemLibrary.h>
#include <Components/BoxComponent.h>
#include <Kismet/GameplayStatics.h>
#include <Components/SkeletalMeshComponent.h>
#include <PhysicsEngine/PhysicsHandleComponent.h>
#include <Components/WidgetInteractionComponent.h>
#include <EyeTrackerFunctionLibrary.h>
#include <IEyeTracker.h>
#include <Kismet/KismetMathLibrary.h>

// Sets default values
AVRPlayer::AVRPlayer()
	: FloatingPawnMovement( nullptr )
	, CapsuleCollider( nullptr )
	, DistanceGrabRange( 150 )
	, VROrigin( nullptr )
	, Camera( nullptr )
	, InputMappingContexts()
	, MoveForwardActionLeft( nullptr )
	, MoveRightActionLeft( nullptr )
	, MoveForwardActionRight( nullptr )
	, MoveRightActionRight( nullptr )
	, ToggleSprintAction( nullptr )
	, SnapTurnActionLeft( nullptr )
	, SnapTurnActionRight( nullptr )
	, MouseRotateAction( nullptr )
	, ToggleFPSAction( nullptr )
	, LeftController( nullptr )
	, RightController( nullptr )
	, SnapTurnAmount( 30 )
	, bMoveWithLeftJoystick( true )
	, bUseTeleportMovement( false )
	, MoveInputStrenght( 0 )
	, MoveSpeed( 0 )
	, WalkSpeed( 40 )
	, SprintSpeed( 60 )
	, bIsSprinting( false )
	, MaxStamina( 50 )
	, CurrentStamina( MaxStamina )
	, StaminaGain( 5 )
	, StaminaDrain( 8 )
	, StaminaTimer( 0 )
	, StaminaUpdateInterval( 1 )
	, StaminaNeededToSprint( 0 )
	, StaminaNeededWhenRecovering( 40 )
	, BagOrigin( nullptr )
	, BagMesh( nullptr )
	, BagCollider( nullptr )
	, StolenObjects()
	, bHasWon( false )
	, ValuablesToSteal( 0 )
	, ObjectsNeededToFinish( 0 )
	, NearStealableSphere( nullptr )
	, InventoryItems()
	, InventoryItemRotations()
	, bUseHapticFeedback( true )
	, NearNeededStealableFeedback( nullptr )
	, NearStealableFeedback( nullptr )
	, NearPuzzleFeedback( nullptr )
	, HitHeadFeedback( nullptr )
	, HitHandFeedback( nullptr )
	, TimeBetweenSteps( 1 )
	, StepTimer( 0 )
	, DistanceBetweenSteps( 200 )
	, LastStepLocation( 0 )
	, FeetCollider( nullptr )
	, bUseEyeTrackingForMenus( false )
	, EyeWidgetInteractor( nullptr )
	, HealthPoints( 2 )
	, HitTeleportPoint( nullptr )
	, HitTeleportBlueprintClass( nullptr )
	, bIsVR( false )
	, bHasResetLocation( false )
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>( TEXT( "FloatingPawn" ) );

	CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "BodyCollider" ) );
	SetRootComponent( CapsuleCollider );
	CapsuleCollider->SetCollisionProfileName( "BlockAllDynamic" );
	CapsuleCollider->SetCollisionObjectType( ECC_PhysicsBody );
	CapsuleCollider->SetSimulatePhysics( true );
	CapsuleCollider->BodyInstance.bLockXRotation = true;
	CapsuleCollider->BodyInstance.bLockYRotation = true;
	CapsuleCollider->BodyInstance.bLockZRotation = true;

	VROrigin = CreateDefaultSubobject<USceneComponent>( TEXT( "VROrigin" ) );
	VROrigin->SetupAttachment( CapsuleCollider );

	Camera = CreateDefaultSubobject<UCameraComponent>( TEXT( "Camera" ) );
	Camera->SetupAttachment( VROrigin );

	{
		LeftController = CreateDefaultSubobject<UVRControllerComponent>( TEXT( "LeftController" ) );
		LeftController->SetupAttachment( VROrigin );
		LeftController->SetCollisionProfileName( "NoCollision" );
		LeftController->SetIsRightController( false );

		UMotionControllerComponent* MotionControllerComponent = CreateDefaultSubobject<UMotionControllerComponent>( TEXT( "LeftMotionController" ) );
		MotionControllerComponent->SetupAttachment( LeftController );
		MotionControllerComponent->MotionSource = "Left";
		MotionControllerComponent->PlayerIndex  = 0;
		LeftController->SetController( MotionControllerComponent );

		USkeletalMeshComponent* SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>( TEXT( "LeftHand" ) );
		SkeletalMesh->SetupAttachment( MotionControllerComponent );
		SkeletalMesh->SetCollisionProfileName( "VRHand" );
		LeftController->SetHandMesh( SkeletalMesh );
		SkeletalMesh->bTraceComplexOnMove = true;

		USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>( TEXT( "LeftGrabSphere" ) );
		SphereComponent->SetupAttachment( SkeletalMesh );
		SphereComponent->SetCollisionProfileName( "OverlapAllDynamic" );
		LeftController->SetGrabSphere( SphereComponent );

		UPhysicsHandleComponent* PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>( TEXT( "LeftPhysicsHandle" ) );
		LeftController->SetPhysicsHandle( PhysicsHandle );

		UWidgetInteractionComponent* WidgetInteractionComponent = CreateDefaultSubobject<UWidgetInteractionComponent>( TEXT( "LeftWidgetInteraction" ) );
		WidgetInteractionComponent->SetupAttachment( SkeletalMesh, "IndexFingerTip_Left" );
		LeftController->SetWidgetInteractionComponent( WidgetInteractionComponent );
	}

	{
		RightController = CreateDefaultSubobject<UVRControllerComponent>( TEXT( "RightController" ) );
		RightController->SetupAttachment( VROrigin );
		RightController->SetCollisionProfileName( "NoCollision" );
		RightController->SetIsRightController( true );

		UMotionControllerComponent* MotionControllerComponent = CreateDefaultSubobject<UMotionControllerComponent>( TEXT( "RightMotionController" ) );
		MotionControllerComponent->SetupAttachment( RightController );
		MotionControllerComponent->MotionSource = "Right";
		MotionControllerComponent->PlayerIndex  = 0;
		RightController->SetController( MotionControllerComponent );

		USkeletalMeshComponent* SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>( TEXT( "RightHand" ) );
		SkeletalMesh->SetupAttachment( MotionControllerComponent );
		SkeletalMesh->SetCollisionProfileName( "VRHand" );
		RightController->SetHandMesh( SkeletalMesh );
		SkeletalMesh->bTraceComplexOnMove = true;

		USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>( TEXT( "RightGrabSphere" ) );
		SphereComponent->SetupAttachment( SkeletalMesh );
		SphereComponent->SetCollisionProfileName( "OverlapAllDynamic" );
		RightController->SetGrabSphere( SphereComponent );

		UPhysicsHandleComponent* PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>( TEXT( "RightPhysicsHandle" ) );
		RightController->SetPhysicsHandle( PhysicsHandle );

		UWidgetInteractionComponent* WidgetInteractionComponent = CreateDefaultSubobject<UWidgetInteractionComponent>( TEXT( "RightWidgetInteraction" ) );
		WidgetInteractionComponent->SetupAttachment( SkeletalMesh, "IndexFingerTip_Right" );
		RightController->SetWidgetInteractionComponent( WidgetInteractionComponent );
	}

	AutoReceiveInput  = EAutoReceiveInput::Player0;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoPossessAI     = EAutoPossessAI::Disabled;

	{
		BagOrigin = CreateDefaultSubobject<USceneComponent>( TEXT( "BagOrigin" ) );
		BagOrigin->SetupAttachment( CapsuleCollider );

		BagMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "BagMesh" ) );
		BagMesh->SetupAttachment( BagOrigin );
		BagMesh->SetCollisionProfileName( "NoCollision" );

		BagCollider = CreateDefaultSubobject<UBoxComponent>( TEXT( "BagCollider" ) );
		BagCollider->SetupAttachment( BagMesh );
		BagCollider->SetCollisionProfileName( "OverlapAll" );
	}

	NearStealableSphere = CreateDefaultSubobject<USphereComponent>( TEXT( "NearStealableSphere" ) );
	NearStealableSphere->SetupAttachment( CapsuleCollider );
	NearStealableSphere->SetCollisionProfileName( "OverlapAllDynamic" );

	{
		InventoryItems.Add( CreateDefaultSubobject<UChildActorComponent>( TEXT( "Clipboard" ) ) );
		InventoryItems.Last()->SetupAttachment( BagOrigin );

		InventoryItems.Add( CreateDefaultSubobject<UChildActorComponent>( TEXT( "RakeTool" ) ) );
		InventoryItems.Last()->SetupAttachment( BagOrigin );

		InventoryItems.Add( CreateDefaultSubobject<UChildActorComponent>( TEXT( "PickingTool" ) ) );
		InventoryItems.Last()->SetupAttachment( BagOrigin );

		InventoryItems.Add( CreateDefaultSubobject<UChildActorComponent>( TEXT( "TensionTool" ) ) );
		InventoryItems.Last()->SetupAttachment( BagOrigin );

		InventoryItems.Add( CreateDefaultSubobject<UChildActorComponent>( TEXT( "Flashlight" ) ) );
		InventoryItems.Last()->SetupAttachment( BagOrigin );
	}

	FeetCollider = CreateDefaultSubobject<UBoxComponent>( TEXT( "FeetCollider" ) );
	FeetCollider->SetupAttachment( CapsuleCollider );
	FeetCollider->SetCollisionProfileName( "OverlapAllDynamic" );

	EyeWidgetInteractor = CreateDefaultSubobject<UWidgetInteractionComponent>( TEXT( "WidgetInteraction" ) );
	EyeWidgetInteractor->SetupAttachment( Camera );

	PerceptionStimSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>( TEXT( "StimSource" ) );
	PerceptionStimSource->Activate();

	OverrideInputComponentClass = UEnhancedInputComponent::StaticClass();
}

// Called when the game starts or when spawned
void AVRPlayer::BeginPlay()
{
	Super::BeginPlay();

	MoveSpeed      = WalkSpeed;
	CurrentStamina = MaxStamina;

	for( UChildActorComponent* ChildActorComponent : InventoryItems )
	{
		AActor* ChildActor = ChildActorComponent->GetChildActor();

		Cast<UPrimitiveComponent>( ChildActor->GetRootComponent() )->SetCollisionResponseToAllChannels( ECR_Overlap );
		InventoryItemRotations.Add( ChildActor->GetActorRotation() );
		InventoryItemRotations.Last().Yaw -= Camera->GetComponentRotation().Yaw;
	}

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass( GetWorld(), AActor::StaticClass(), OutActors );
	for( AActor* Actor : OutActors )
	{
		UStealableComponent* StealableComponent = Cast<UStealableComponent>( Actor->GetComponentByClass( UStealableComponent::StaticClass() ) );

		if( StealableComponent )
		{
			if( StealableComponent->IsNeeded() )
				ObjectsNeededToFinish++;
			else
				ValuablesToSteal++;
		}
		else if( Actor->GetClass() == HitTeleportBlueprintClass )
			HitTeleportPoint = Actor;
	}

	bIsVR = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
	if( bIsVR )
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin( EHMDTrackingOrigin::Stage );
		const FVector& ActorLocation = GetActorLocation();
		const FVector  NewLocation( ActorLocation.X, ActorLocation.Y, ActorLocation.Z - CapsuleCollider->GetUnscaledCapsuleHalfHeight() );
		VROrigin->SetWorldLocation( NewLocation );
	}
	else
	{
		const FVector& ActorLocation = GetActorLocation();
		const FVector  NewLocation( ActorLocation.X, ActorLocation.Y, ActorLocation.Z + CapsuleCollider->GetUnscaledCapsuleHalfHeight() );
		VROrigin->SetWorldLocation( NewLocation );
	}

	ULocalPlayer* Player = GetWorld()->GetFirstLocalPlayerFromController();
	if( Player )
	{
		UEnhancedInputLocalPlayerSubsystem* SubSystem = Player->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

		if( SubSystem )
		{
			for( UInputMappingContext* Context : InputMappingContexts )
				SubSystem->AddMappingContext( Context, 0 );
		}
	}

	AActor* MSActor = UGameplayStatics::GetActorOfClass( GetWorld(), AMotionSicknessHelper::StaticClass() );
	if( MSActor )
		MotionSicknessHelper = dynamic_cast<AMotionSicknessHelper*>( MSActor );
	else if( GEngine )
		GEngine->AddOnScreenDebugMessage( -1, 5, FColor::Red, "VRPlayer: Could not find MotionSicknessHelper" );

	EyeWidgetInteractor->SetActive( bUseEyeTrackingForMenus );

	BagCollider->OnComponentBeginOverlap.AddDynamic( this, &AVRPlayer::OnMoneyBagBeginOverlap );
	NearStealableSphere->OnComponentBeginOverlap.AddDynamic( this, &AVRPlayer::AVRPlayer::OnNearBeginOverlap );
	FeetCollider->OnComponentBeginOverlap.AddDynamic( this, &AVRPlayer::AVRPlayer::OnFeetBeginOverlap );
}

void AVRPlayer::MoveForward( const FInputActionInstance& Instance ) { UPlayerMovement::MoveForward( Instance, this ); }
void AVRPlayer::MoveRight( const FInputActionInstance& Instance ) { UPlayerMovement::MoveRight( Instance, this ); }
void AVRPlayer::Teleport( const FInputActionInstance& Instance ) { UPlayerMovement::Teleport( Instance, this ); }
void AVRPlayer::ToggleSprint( const FInputActionInstance& Instance ) { UPlayerMovement::ToggleSprint( Instance, this ); }
void AVRPlayer::SnapTurn( const FInputActionInstance& Instance ) { UPlayerMovement::SnapTurn( Instance, this ); }
void AVRPlayer::MouseRotation( const FInputActionInstance& Instance ) { UPlayerMovement::MouseRotation( Instance, this ); }

void AVRPlayer::LeftGrabOrDrop( const FInputActionInstance& Instance )
{
	LeftController->GrabOrDrop( Instance, this );

	if( LeftController->GetGrabbedComponent() && LeftController->GetGrabbedComponent()->ComponentHasTag( "Van" ) && ObjectsNeededToFinish == 0 )
		OnWin();
}

void AVRPlayer::LeftInteract( const FInputActionInstance& Instance ) { LeftController->Interact( Instance ); }

void AVRPlayer::RightGrabOrDrop( const FInputActionInstance& Instance )
{
	RightController->GrabOrDrop( Instance, this );

	if( RightController->GetGrabbedComponent() && RightController->GetGrabbedComponent()->ComponentHasTag( "Van" ) && ObjectsNeededToFinish == 0 )
		OnWin();
}

void AVRPlayer::RightInteract( const FInputActionInstance& Instance ) { RightController->Interact( Instance ); }

void AVRPlayer::LeftGrasp( const FInputActionInstance& Instance ) { LeftController->Grasp( Instance ); }
void AVRPlayer::LeftIndexCurl( const FInputActionInstance& Instance ) { LeftController->IndexCurl( Instance ); }
void AVRPlayer::RightGrasp( const FInputActionInstance& Instance ) { RightController->Grasp( Instance ); }
void AVRPlayer::RightIndexCurl( const FInputActionInstance& Instance ) { RightController->IndexCurl( Instance ); }

void AVRPlayer::ToggleFPS( const FInputActionInstance& Instance )
{
	if( Instance.GetValue().Get<bool>() )
	{
		UKismetSystemLibrary::ExecuteConsoleCommand( GetWorld(), "stat fps" );
		UKismetSystemLibrary::ExecuteConsoleCommand( GetWorld(), "stat unit" );
	}
}

void AVRPlayer::OnMoneyBagBeginOverlap( UPrimitiveComponent*   OverlappedComponent
                                        , AActor*              OtherActor
                                        , UPrimitiveComponent* OtherComp
                                        , int32                OtherBodyIndex
                                        , bool                 bFromSweep
                                        , const FHitResult&    SweepResult )
{
	Super::NotifyActorBeginOverlap( OtherActor );

	UStealableComponent* StealableComponent = Cast<UStealableComponent>( OtherActor->GetComponentByClass( UStealableComponent::StaticClass() ) );
	if( !StealableComponent )
		return;

	if( StealableComponent == LeftController->GetGrabbedComponent() )
		LeftController->ClearGrabbedComponent();
	else if( StealableComponent == RightController->GetGrabbedComponent() )
		RightController->ClearGrabbedComponent();

	for( UStealableComponent* StolenObject : StolenObjects )
	{
		if( StolenObject == StealableComponent )
			return;
	}

	if( !StealableComponent->IsNeeded() )
		StolenObjects.Add( StealableComponent );
	else
		ObjectsNeededToFinish--;

	OnSteal( StealableComponent->GetOwner() );
	StealableComponent->GetOwner()->SetActorHiddenInGame( true );
	StealableComponent->GetOwner()->DisableComponentsSimulatePhysics();
	StealableComponent->GetOwner()->SetActorEnableCollision( false );
}

void AVRPlayer::OnNearBeginOverlap( UPrimitiveComponent*   OverlappedComponent
                                    , AActor*              OtherActor
                                    , UPrimitiveComponent* OtherComp
                                    , int32                OtherBodyIndex
                                    , bool                 bFromSweep
                                    , const FHitResult&    SweepResult )
{
	Super::NotifyActorBeginOverlap( OtherActor );

	if( !bUseHapticFeedback )
		return;

	APlayerController* PlayerController = Cast<APlayerController>( GetController() );
	if( !PlayerController )
		return;

	UStealableComponent* StealableComponent = Cast<UStealableComponent>( OtherActor->GetComponentByClass( UStealableComponent::StaticClass() ) );
	if( StealableComponent )
	{
		if( StealableComponent->IsNeeded() )
		{
			if( NearNeededStealableFeedback )
			{
				OnNearNeededStealable();
				PlayerController->PlayHapticEffect( NearNeededStealableFeedback, EControllerHand::Left );
				PlayerController->PlayHapticEffect( NearNeededStealableFeedback, EControllerHand::Right );
			}
		}
		else
		{
			if( NearStealableFeedback )
			{
				OnNearStealable();
				PlayerController->PlayHapticEffect( NearStealableFeedback, EControllerHand::Left );
				PlayerController->PlayHapticEffect( NearStealableFeedback, EControllerHand::Right );
			}
		}
	}

	if( NearPuzzleFeedback )
	{
		for( UClass* PuzzleObjectClass : PuzzleObjectClasses )
		{
			if( PuzzleObjectClass == OtherActor->GetClass() )
			{
				PlayerController->PlayHapticEffect( NearPuzzleFeedback, EControllerHand::HMD );
				break;
			}
		}
	}
}

void AVRPlayer::OnFeetBeginOverlap( UPrimitiveComponent*   OverlappedComponent
                                    , AActor*              OtherActor
                                    , UPrimitiveComponent* OtherComp
                                    , int32                OtherBodyIndex
                                    , bool                 bFromSweep
                                    , const FHitResult&    SweepResult )
{
	Super::NotifyActorBeginOverlap( OtherActor );

	OnStepOnActor( OtherActor );
}

void AVRPlayer::UpdateMovement( float DeltaTime )
{
	FVector ForwardVector = Camera->GetForwardVector();
	ForwardVector.Z       = 0;
	ForwardVector.Normalize();

	FVector RightVector = Camera->GetRightVector();
	RightVector.Z       = 0;
	RightVector.Normalize();

	FVector Movement = MoveInputStrenght.X * ForwardVector;
	Movement += MoveInputStrenght.Y * RightVector;
	if( !bIsVR )
		Movement.Normalize();

	if( !bUseTeleportMovement )
		FloatingPawnMovement->AddInputVector( Movement * MoveSpeed * DeltaTime );

	FHitResult                  HitResult{};
	FVector                     Offset( 0, 0, 1000 );
	FCollisionObjectQueryParams Params;
	Params.AddObjectTypesToQuery( ECC_WorldStatic );
	GetWorld()->LineTraceSingleByObjectType( HitResult, GetActorLocation(), GetActorLocation() - Offset, Params );

	Offset.Z = CapsuleCollider->GetUnscaledCapsuleHalfHeight() + 10;
	if( ( HitResult.Location - GetActorLocation() ).Length() < 500.0f )
		SetActorLocation( HitResult.Location + Offset );

	if( HitResult.GetActor() )
		UpdateStepSound( DeltaTime, HitResult.GetActor()->Tags );

	if( MotionSicknessHelper )
		MotionSicknessHelper->UpdateVignette( Movement.Length() );

	MoveInputStrenght = FVector::ZeroVector;
}

void AVRPlayer::UpdateStamina( float DeltaTime )
{
	StaminaTimer += DeltaTime;

	if( StaminaTimer < StaminaUpdateInterval )
		return;

	StaminaTimer = 0;
	if( bIsSprinting )
	{
		CurrentStamina -= StaminaDrain;

		if( CurrentStamina < 0 )
		{
			CurrentStamina        = 0;
			StaminaNeededToSprint = StaminaNeededWhenRecovering;
			MoveSpeed             = WalkSpeed;
		}
	}
	else
	{
		CurrentStamina += StaminaGain;

		if( CurrentStamina > MaxStamina )
		{
			CurrentStamina        = MaxStamina;
			StaminaNeededToSprint = 0;
		}
	}
}

void AVRPlayer::ReportSound()
{
	UAISense_Hearing::ReportNoiseEvent( GetWorld(), GetActorLocation(), 1.0f, this );
}

void AVRPlayer::TeleportToHitTeleportPoint()
{
	FVector HitTeleportPointLocation = HitTeleportPoint->GetActorLocation();
	SetActorLocation( HitTeleportPointLocation );

	FVector CameraToActor = GetActorLocation() - Camera->GetComponentLocation();
	CameraToActor.Z       = 0;
	AddActorWorldOffset( CameraToActor );

	HitTeleportPoint->Destroy();
	HitTeleportPoint = nullptr;
}

FString AVRPlayer::DisplayMoneyTaken()
{
	float TotalMoney = 0;
	for( int i = 0; i < StolenObjects.Num(); i++ )
	{
		TotalMoney += StolenObjects[i]->MoneyValue();
	}
	FString a = "Total Worth = " + FString::FromInt( TotalMoney );

	FString DisplayString = "You stole " + FString::FromInt( StolenObjects.Num() ) + " / " + FString::FromInt( ValuablesToSteal ) + "\n" + a;

	return DisplayString;
}

void AVRPlayer::UpdateCapsuleLocation()
{
	FVector ActorToCamera = Camera->GetComponentLocation() - GetActorLocation();
	ActorToCamera.Z       = 0;

	AddActorWorldOffset( ActorToCamera );
	VROrigin->AddWorldOffset( -ActorToCamera );
	Cast<UPrimitiveComponent>( GetRootComponent() )->SetPhysicsLinearVelocity( FVector( 0 ) );
}

void AVRPlayer::UpdateDynamicHeight()
{
	const float Height = Camera->GetComponentLocation().Z - VROrigin->GetComponentLocation().Z;
	CapsuleCollider->SetCapsuleHalfHeight( Height / 2 );

	const FVector VROriginLocation = VROrigin->GetComponentLocation();
	const FVector NewLocation( VROriginLocation.X, VROriginLocation.Y, GetActorLocation().Z - CapsuleCollider->GetScaledCapsuleHalfHeight() - 10 );
	VROrigin->SetWorldLocation( NewLocation );
}

void AVRPlayer::UpdateBagLocationAndRotation()
{
	FVector NewLocation = BagOrigin->GetComponentLocation();
	NewLocation.Z       = Camera->GetComponentLocation().Z;

	FRotator NewRotation( 0 );
	NewRotation.Yaw = Camera->GetComponentRotation().Yaw;

	BagOrigin->SetWorldLocationAndRotation( NewLocation, NewRotation );
}

void AVRPlayer::UpdateInventoryItemLocations()
{
	TArray<UChildActorComponent*> HeldItems;

	if( LeftController->GetGrabbedComponent() )
	{
		const AActor* GrabbedActor = LeftController->GetGrabbedComponent()->GetOwner();

		for( UChildActorComponent* ChildActorComponent : InventoryItems )
		{
			if( ChildActorComponent->GetChildActor() == GrabbedActor )
				HeldItems.Add( ChildActorComponent );
		}
	}

	if( RightController->GetGrabbedComponent() )
	{
		const AActor* GrabbedActor = RightController->GetGrabbedComponent()->GetOwner();

		for( UChildActorComponent* ChildActorComponent : InventoryItems )
		{
			if( ChildActorComponent->GetChildActor() == GrabbedActor )
				HeldItems.Add( ChildActorComponent );
		}
	}

	const float CameraYaw = Camera->GetComponentRotation().Yaw;
	for( int i = 0; i < InventoryItems.Num(); ++i )
	{
		UChildActorComponent* ChildActorComponent = InventoryItems[i];

		if( !HeldItems.Contains( ChildActorComponent ) )
		{
			AActor* ChildActor = ChildActorComponent->GetChildActor();

			ChildActor->SetActorLocation( ChildActorComponent->GetComponentLocation() );
			ChildActor->SetActorRotation( FRotator( InventoryItemRotations[i].Pitch, CameraYaw + InventoryItemRotations[i].Yaw, InventoryItemRotations[i].Roll ) );
		}
	}
}

void AVRPlayer::UpdateStepSound( float DeltaTime, TArray<FName>& FloorTags )
{
	if( MoveInputStrenght.Length() == 0 )
		return;

	StepTimer += DeltaTime;
	const FVector& ActorLocation = GetActorLocation();

	if( ( ActorLocation - LastStepLocation ).Length() > DistanceBetweenSteps || StepTimer > TimeBetweenSteps )
	{
		StepTimer        = 0;
		LastStepLocation = ActorLocation;
		OnStepTaken( FloorTags );
	}
}

void AVRPlayer::UpdateMenuEyeTracking()
{
	if( !bUseEyeTrackingForMenus )
		return;

	FEyeTrackerGazeData OutGazeData{};
	if( !UEyeTrackerFunctionLibrary::GetGazeData( OutGazeData ) )
		return;

	const FRotator EyeTrackingRotation = UKismetMathLibrary::FindLookAtRotation( FVector::Zero(), OutGazeData.GazeDirection );
	EyeWidgetInteractor->SetWorldRotation( EyeTrackingRotation );
}

// Called every frame 
void AVRPlayer::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if( !bHasResetLocation )
	{
		FVector ActorToVROrigin = VROrigin->GetComponentLocation() - GetActorLocation();
		ActorToVROrigin.Z       = 0;

		if( ActorToVROrigin.Length() > 0 )
		{
			bHasResetLocation = true;
			AddActorWorldOffset( ActorToVROrigin );
		}
	}

	UpdateMovement( DeltaTime );
	UpdateStamina( DeltaTime );
	UpdateCapsuleLocation();
	UpdateBagLocationAndRotation();
	UpdateInventoryItemLocations();
	UpdateMenuEyeTracking();

	if( bIsVR )
		UpdateDynamicHeight();
}

// Called to bind functionality to input
void AVRPlayer::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	Super::SetupPlayerInputComponent( PlayerInputComponent );

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>( PlayerInputComponent );

	{
		if( LeftController->GetGrabAction() )
			Input->BindAction( LeftController->GetGrabAction(), ETriggerEvent::Triggered, this, &AVRPlayer::LeftGrabOrDrop );
		if( LeftController->GetInteractAction() )
			Input->BindAction( LeftController->GetInteractAction(), ETriggerEvent::Triggered, this, &AVRPlayer::LeftInteract );

		if( LeftController->GetGraspAction() )
		{
			Input->BindAction( LeftController->GetGraspAction(), ETriggerEvent::Triggered, this, &AVRPlayer::LeftGrasp );
			Input->BindAction( LeftController->GetGraspAction(), ETriggerEvent::Completed, this, &AVRPlayer::LeftGrasp );
			Input->BindAction( LeftController->GetGraspAction(), ETriggerEvent::Canceled, this, &AVRPlayer::LeftGrasp );
		}
		if( LeftController->GetIndexCurlAction() )
		{
			Input->BindAction( LeftController->GetIndexCurlAction(), ETriggerEvent::Triggered, this, &AVRPlayer::LeftIndexCurl );
			Input->BindAction( LeftController->GetIndexCurlAction(), ETriggerEvent::Completed, this, &AVRPlayer::LeftIndexCurl );
			Input->BindAction( LeftController->GetIndexCurlAction(), ETriggerEvent::Canceled, this, &AVRPlayer::LeftIndexCurl );
		}
	}

	{
		if( RightController->GetGrabAction() )
			Input->BindAction( RightController->GetGrabAction(), ETriggerEvent::Triggered, this, &AVRPlayer::RightGrabOrDrop );
		if( RightController->GetInteractAction() )
			Input->BindAction( RightController->GetInteractAction(), ETriggerEvent::Triggered, this, &AVRPlayer::RightInteract );

		if( RightController->GetGraspAction() )
		{
			Input->BindAction( RightController->GetGraspAction(), ETriggerEvent::Triggered, this, &AVRPlayer::RightGrasp );
			Input->BindAction( RightController->GetGraspAction(), ETriggerEvent::Completed, this, &AVRPlayer::RightGrasp );
			Input->BindAction( RightController->GetGraspAction(), ETriggerEvent::Canceled, this, &AVRPlayer::RightGrasp );
		}
		if( RightController->GetIndexCurlAction() )
		{
			Input->BindAction( RightController->GetIndexCurlAction(), ETriggerEvent::Triggered, this, &AVRPlayer::RightIndexCurl );
			Input->BindAction( RightController->GetIndexCurlAction(), ETriggerEvent::Completed, this, &AVRPlayer::RightIndexCurl );
			Input->BindAction( RightController->GetIndexCurlAction(), ETriggerEvent::Canceled, this, &AVRPlayer::RightIndexCurl );
		}
	}

	if( MoveForwardActionLeft )
	{
		Input->BindAction( MoveForwardActionLeft, ETriggerEvent::Triggered, this, &AVRPlayer::MoveForward );
		Input->BindAction( MoveForwardActionLeft, ETriggerEvent::Completed, this, &AVRPlayer::Teleport );
	}
	if( MoveRightActionLeft )
	{
		Input->BindAction( MoveRightActionLeft, ETriggerEvent::Triggered, this, &AVRPlayer::MoveRight );
		Input->BindAction( MoveRightActionLeft, ETriggerEvent::Completed, this, &AVRPlayer::Teleport );
	}

	if( MoveForwardActionRight )
	{
		Input->BindAction( MoveForwardActionRight, ETriggerEvent::Triggered, this, &AVRPlayer::MoveForward );
		Input->BindAction( MoveForwardActionRight, ETriggerEvent::Completed, this, &AVRPlayer::Teleport );
	}
	if( MoveRightActionRight )
	{
		Input->BindAction( MoveRightActionRight, ETriggerEvent::Triggered, this, &AVRPlayer::MoveRight );
		Input->BindAction( MoveRightActionRight, ETriggerEvent::Completed, this, &AVRPlayer::Teleport );
	}

	if( ToggleSprintAction )
		Input->BindAction( ToggleSprintAction, ETriggerEvent::Triggered, this, &AVRPlayer::ToggleSprint );

	if( SnapTurnActionLeft )
		Input->BindAction( SnapTurnActionLeft, ETriggerEvent::Triggered, this, &AVRPlayer::SnapTurn );
	if( SnapTurnActionRight )
		Input->BindAction( SnapTurnActionRight, ETriggerEvent::Triggered, this, &AVRPlayer::SnapTurn );

	if( MouseRotateAction )
		Input->BindAction( MouseRotateAction, ETriggerEvent::Triggered, this, &AVRPlayer::MouseRotation );

	if( ToggleFPSAction )
		Input->BindAction( ToggleFPSAction, ETriggerEvent::Triggered, this, &AVRPlayer::ToggleFPS );
}

void AVRPlayer::PlayHapticEffect( UHapticFeedbackEffect_Base* HapticEffect, EControllerHand Hand, float Scale, bool bLoop )
{
	if( !bUseHapticFeedback )
		return;

	APlayerController* PlayerController = Cast<APlayerController>( GetController() );
	if( !PlayerController )
		return;

	PlayerController->PlayHapticEffect( HapticEffect, Hand, Scale, bLoop );
}

void AVRPlayer::Hit()
{
	if( HealthPoints == 0 )
		return;

	APlayerController* PlayerController = Cast<APlayerController>( GetController() );
	if( !PlayerController )
		return;

	if( HitHandFeedback )
	{
		PlayerController->PlayHapticEffect( HitHandFeedback, EControllerHand::Left );
		PlayerController->PlayHapticEffect( HitHandFeedback, EControllerHand::Right );
	}

	if( HitHeadFeedback )
		PlayerController->PlayHapticEffect( HitHeadFeedback, EControllerHand::HMD );

	if( HitTeleportPoint )
	{
		OnFirstHitByGreta();
		return;
	}

	HealthPoints--;
	if( HealthPoints == 0 )
		OnLose();
}
