// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PSJamJan/Interfaces/PickUpInterface.h"
#include "GameFramework/Actor.h"
#include "FlashLight.generated.h"

UCLASS()
class PSJAMJAN_API AFlashLight : public AActor, public IPickUpInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlashLight();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
