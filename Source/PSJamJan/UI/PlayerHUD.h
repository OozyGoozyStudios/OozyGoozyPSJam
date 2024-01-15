// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "UI.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class PSJAMJAN_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
public:
	APlayerHUD();

	void BeginPlay();
	void Tick(float DeltaSeconds);
	TSubclassOf<class UUserWidget> UserInterfaceClass;
	class UUI* UserInterface;

};
