// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QuestDataAsset.generated.h"

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
	CollectItem,
	CompleteInteraction

};

USTRUCT(BlueprintType)
struct FQuest
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Quest")
	FString Quest;
	UPROPERTY(EditAnywhere, Category = "Quest")
	FString QuestText;
	UPROPERTY(EditAnywhere, Category = "Quest")
	int QuestOrder;
	//if multiple items needed to complete objective
	UPROPERTY(EditAnywhere, Category = "Quest")
	int NumberOfObjectives = 1;
	UPROPERTY(EditAnywhere, Category = "Quest")
	EObjectiveType Objectivetype;
};

UCLASS()
class PSJAMJAN_API UQuestDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Missions")
	TArray<FQuest> QuestVector;
};
