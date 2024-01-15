// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "QuestWidget.generated.h"

/**
 * 
 */
UCLASS()
class PSJAMJAN_API UQuestWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* QuestText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Progression;
};
