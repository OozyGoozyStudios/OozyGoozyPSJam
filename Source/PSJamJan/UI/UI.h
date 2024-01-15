// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UObjectGlobals.h"
#include "../PSJamJanCharacter.h"
#include "../Quests/QuestDataAsset.h"
#include "UI.generated.h"

/**
 * 
 */
UCLASS()
class PSJAMJAN_API UUI : public UUserWidget
{
	GENERATED_BODY()
public:
	UUI(const FObjectInitializer& ObjectInitializer);
	void Setup();

	bool Initialize();
	void Mission();
	void UpdateText();
	APlayerController* PlayerController;
	APSJamJanCharacter* Character;
	UQuestDataAsset* QuestData;

	TSubclassOf<class UQuestWidget> QuestClass;
	UQuestWidget* mission;
	UPROPERTY(meta = (BindWidget))
	class USizeBox* QuestBox;

	

};
