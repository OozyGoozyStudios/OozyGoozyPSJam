// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupComponent.h"
#include "../PSJamJanCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values for this component's properties
UPickupComponent::UPickupComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPickupComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPickupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPickupComponent::AttachWeapon(APSJamJanCharacter* TargetCharacter, AActor* Weapon)
{
	APSJamJanCharacter* Character = TargetCharacter;
	if (Character == nullptr)
	{
		return;
	}

	TArray<UActorComponent*> FlashLightPosArray = Character->GetComponentsByClass(USkeletalMeshComponent::StaticClass());
	
	for(auto& FL : FlashLightPosArray)
	{
		FlashLightPos = Cast<USkeletalMeshComponent>(FL);
	}
	
	if (FlashLightPos == nullptr)
	{
		return;
	}
	const USkeletalMeshSocket* HandSocket = FlashLightPos->GetSocketByName(FName("FlashLight"));

	if (HandSocket && Weapon)
	{
		HandSocket->AttachActor(Weapon, FlashLightPos);
		Weapon->SetOwner(Character);
		Weapon->SetActorEnableCollision(true);

		
	}
	
}

