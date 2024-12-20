// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "FlashLight.h"
#include "Components/SpotLightComponent.h"
#include "Quests/QuestDataAsset.h"
#include "PSJamJanCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

UCLASS()
class APSJamJanCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* InteractMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ToggleLightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* RechargeLightAction;

	/** Interact Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ExitInteractAction;

	FHitResult HitResult;

	bool Interatable = false;

	bool InteractPressed = false;

	class UPickupComponent* Equipped;

	bool FlashLightEquipped = false;

	bool LightOnOff = false;

	USpotLightComponent* SpotLight;

	FTimerHandle LightTimerhandle;

	UPROPERTY(EditAnywhere)
	float StartLightCharge = 60.0f;

	UPROPERTY(VisibleAnywhere)
	float CurrentLightCharge;

	UPROPERTY(VisibleAnywhere)
	float BrightnessPercentage;

	UPROPERTY(EditAnywhere)
	float RateOfDecayLight = 0.1f;

	UPROPERTY(EditAnywhere)
	float MaxIntensity = 100.0f;

	//Quest Variables
	UPROPERTY(VisibleAnywhere)
	int CollectedItems = 0;
	UPROPERTY(VisibleAnywhere)
	int CurrentQuestNumber = 0;


public:
	APSJamJanCharacter();
	int GetCurrentQuestNumber()
	{
		return CurrentQuestNumber;
	}
	void AddToQuestNumber(int add)
	{
		
		CurrentQuestNumber += add;
	}
	int GetCollectedItems()
	{
		return CollectedItems;
	}
	void SetCollectedItems(int CollectedNum)
	{
		CollectedItems = CollectedNum;
	}
protected:
	virtual void BeginPlay();
	virtual void Tick( float DeltaSeconds ) override;
public:

	
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();

	void ShootRay();

	void ToggleLight();
	void StopLight();

	void ToggleFlashLight(bool OnOff);
	USpotLightComponent* GetLight();
	
	void StartTimer();
	void StopTimer();
	void SubtractTime();
	void AddTime();
	void SetBrightness();
	UQuestDataAsset* GetQuestDataAsset();
protected:
	/** Called for movement input 
	void Move(const FInputActionValue& Value);*/

	/** Called for looking input 
	void Look(const FInputActionValue& Value);*/

	void Interact();
	void StopInteract();

	void StartExitInteract();
	void StopExitInteract();

	void AddMappingContext(class UInputMappingContext* Map);
	void RemoveMappingContext(class UInputMappingContext* Map);

	void RechargeLight();
	void StopRechargeLight();


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


};

