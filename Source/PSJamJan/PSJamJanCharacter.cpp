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
#include "Components/PickupComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
	/*Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;*/
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	//Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
	Equipped = CreateDefaultSubobject<UPickupComponent>(TEXT("EquippedComponent"));

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
	CurrentLightCharge = StartLightCharge;
	SpotLight = GetLight();
	if (SpotLight)
	{
		SpotLight->SetVisibility(false);
	}
}

void APSJamJanCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ShootRay();
	if (SpotLight)
	{
		SetBrightness();

	}
	
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

		EnhancedInputComponent->BindAction(ToggleLightAction, ETriggerEvent::Triggered, this, &APSJamJanCharacter::ToggleLight);
		EnhancedInputComponent->BindAction(ToggleLightAction, ETriggerEvent::Completed, this, &APSJamJanCharacter::StopLight);

		//EnhancedInputComponent->BindAction(RechargeLightAction, ETriggerEvent::Triggered, this, &APSJamJanCharacter::RechargeLight);
		EnhancedInputComponent->BindAction(RechargeLightAction, ETriggerEvent::Completed, this, &APSJamJanCharacter::RechargeLight);

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

void APSJamJanCharacter::RechargeLight()
{
	AddTime();
}

void APSJamJanCharacter::StopRechargeLight()
{

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
		if(Hit->Implements<UPickUpInterface>())
		{
			UE_LOG(LogTemp, Warning, TEXT("Hitting a Pickable object"));
			AFlashLight* FlashLight = Cast<AFlashLight>(Hit);
			if (FlashLight)
			{
				Interatable = true;
				if (InteractPressed && Equipped && FlashLight)
				{
					Equipped->AttachWeapon(this, FlashLight);
					InteractPressed = false;
					FlashLightEquipped = true;
					SetCollectedItems(GetCollectedItems() + 1);
					if (!SpotLight)
					{
						SpotLight = GetLight();
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Not Hitting a interactable object"));
			/*Interatable = false;*/
		}
	}
	
}

void APSJamJanCharacter::ToggleLight()
{

}

void APSJamJanCharacter::StopLight()
{
	if (FlashLightEquipped)
	{
		if (LightOnOff == true)
		{
			UE_LOG(LogTemp, Warning, TEXT("Light Off"));
			LightOnOff = false;
			ToggleFlashLight(false);
			StopTimer();
			return;
		}
		if (LightOnOff == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Light On"));
			LightOnOff = true;
			ToggleFlashLight(true);
			StartTimer();
		}
	}
}

void APSJamJanCharacter::ToggleFlashLight(bool OnOff)
{
	SpotLight->SetVisibility(OnOff);
}

USpotLightComponent* APSJamJanCharacter::GetLight()
{
	TArray<UActorComponent*> FlashLightArray = this->GetComponentsByClass(USpotLightComponent::StaticClass());

	for (auto& Light : FlashLightArray)
	{
		return SpotLight = Cast<USpotLightComponent>(Light);
	}
	return nullptr;
}

void APSJamJanCharacter::StartTimer()
{
	
	if (!GetWorld()->GetTimerManager().IsTimerActive(LightTimerhandle))
	{
		
		GetWorld()->GetTimerManager().SetTimer(LightTimerhandle, this, &APSJamJanCharacter::SubtractTime, 1.0f, false, 0.0f);
	}
	else if (GetWorld()->GetTimerManager().IsTimerPaused(LightTimerhandle))
	{
		GetWorld()->GetTimerManager().UnPauseTimer(LightTimerhandle);
	}
}

void APSJamJanCharacter::StopTimer()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(LightTimerhandle))
	{
		GetWorld()->GetTimerManager().PauseTimer(LightTimerhandle);
	}
}

void APSJamJanCharacter::SubtractTime()
{
	if (CurrentLightCharge > 0)
	{
		CurrentLightCharge -= RateOfDecayLight;
		GetWorld()->GetTimerManager().ClearTimer(LightTimerhandle);
		StartTimer();
	}
}

void APSJamJanCharacter::AddTime()
{
	if (CurrentLightCharge < StartLightCharge)
	{
		CurrentLightCharge = UKismetMathLibrary::Min(StartLightCharge, (CurrentLightCharge + 10.0f));
	}
}

void APSJamJanCharacter::SetBrightness()
{
	BrightnessPercentage = (CurrentLightCharge/StartLightCharge) * MaxIntensity;
	SpotLight->SetIntensity(BrightnessPercentage);
}

UQuestDataAsset* APSJamJanCharacter::GetQuestDataAsset()
{
	UQuestDataAsset* MissionDataAsset = LoadObject<UQuestDataAsset>(NULL, TEXT("/Game/FirstPerson/Blueprints/Quests/DA_Quest"));
	return MissionDataAsset;

}
