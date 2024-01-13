// Copyright Epic Games, Inc. All Rights Reserved.

#include "PSJamJanCharacter.h"
#include "PSJamJanProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/InteractionInterface.h"
#include "GameFramework/PlayerController.h"
#include "PSJamJan/Interfaces/PickUpInterface.h"
#include "FlashLight.h"
#include "PSJamJan/TP_WeaponComponent.h"

//////////////////////////////////////////////////////////////////////////
// APSJamJanCharacter

APSJamJanCharacter::APSJamJanCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
	Equipped = CreateDefaultSubobject<UTP_WeaponComponent>(TEXT("EquippedComponent"));
}

void APSJamJanCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			
		}
	}

}

void APSJamJanCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ShootRay();
}

//////////////////////////////////////////////////////////////////////////// Input

void APSJamJanCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APSJamJanCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APSJamJanCharacter::Look);
		
		//Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &APSJamJanCharacter::Interact);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &APSJamJanCharacter::StopInteract);

		// Focused 
		EnhancedInputComponent->BindAction(ExitInteractAction, ETriggerEvent::Triggered, this, &APSJamJanCharacter::StartExitInteract);
		EnhancedInputComponent->BindAction(ExitInteractAction, ETriggerEvent::Completed, this, &APSJamJanCharacter::StopExitInteract);
	}
}


void APSJamJanCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APSJamJanCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APSJamJanCharacter::Interact()
{
	if (Interatable)
	{
		InteractPressed = true;
	}
}

void APSJamJanCharacter::StopInteract()
{
	InteractPressed = false;
}

void APSJamJanCharacter::StartExitInteract()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		PlayerController->SetViewTargetWithBlend(this);
		StopInteract();
		RemoveMappingContext(InteractMappingContext);
		AddMappingContext(DefaultMappingContext);
	}
}

void APSJamJanCharacter::StopExitInteract()
{

}

void APSJamJanCharacter::AddMappingContext(class UInputMappingContext* Map)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(Map, 0);

		}
	}
}

void APSJamJanCharacter::RemoveMappingContext(class UInputMappingContext* Map)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(Map);
		}
	}
}

void APSJamJanCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool APSJamJanCharacter::GetHasRifle()
{
	return bHasRifle;
}

void APSJamJanCharacter::ShootRay()
{
	if (FirstPersonCameraComponent)
	{
		FTransform CameraTransform = FirstPersonCameraComponent->GetComponentTransform();
		FVector CameraLocation = CameraTransform.GetLocation();
		FVector CameraDirection = CameraTransform.GetRotation().GetForwardVector();
		FVector CameraRay = CameraLocation + CameraDirection * 200.0f;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, CameraRay, ECollisionChannel::ECC_Visibility))
		{
			DrawDebugLine(
				GetWorld(),
				CameraLocation,
				HitResult.Location,
				FColor(255, 0, 0),
				false, -1, 0,
				12.333
			);

		}
	}
	AActor* Hit = HitResult.GetActor();
	if (Hit)
	{
		if (Hit->Implements<UInteractionInterface>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Hitting a interactable object"));
			Interatable = true;
			if (InteractPressed)
			{
				APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
				if (PlayerController)
				{
					PlayerController->SetViewTargetWithBlend(Hit);
					StopInteract();
					RemoveMappingContext(DefaultMappingContext);
					AddMappingContext(InteractMappingContext);
				}
			}
		}
		else if(Hit->Implements<UPickUpInterface>())
		{
			if (Cast<AFlashLight>(Hit))
			{
				if (InteractPressed)
				{
					Equipped->AttachWeapon(nullptr);
					InteractPressed = false;
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Not Hitting a interactable object"));
		}
	}
	
}
