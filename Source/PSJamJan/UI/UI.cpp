// Fill out your copyright notice in the Description page of Project Settings.


#include "UI.h"
#include "../Quests/QuestDataAsset.h"
#include "QuestWidget.h"
#include "Components/SizeBox.h"

UUI::UUI(const FObjectInitializer& ObjectInitializer)
	:UUserWidget(ObjectInitializer)
{
	/*static ConstructorHelpers::FClassFinder<UUserWidget> QuestBPClass(TEXT("/Game/FirstPerson/Blueprints/UserInterface/WBP_Quest"));
	if (!ensure(QuestBPClass.Class != nullptr)) return;

	QuestClass = QuestBPClass.Class;*/
}

void UUI::Setup()
{
	this->AddToViewport();
}

bool UUI::Initialize()
{
	bool Success = Super::Initialize();

	//PlayerController = GetOwningPlayer();
	//if (PlayerController == nullptr)
	//{
	//	return false;
	//}
	//Character = Cast<APSJamJanCharacter>(PlayerController->GetCharacter());
	//if (Character == nullptr)
	//{
	//	return false;
	//}
	//QuestData = Character->GetQuestDataAsset();



	return true;
}

void UUI::Mission()
{

	/*mission = CreateWidget<UQuestWidget>(GetWorld(), QuestClass);
	if (!ensure(mission != nullptr)) return;
	UpdateText();
	QuestBox->AddChild(mission);*/

}


void UUI::UpdateText()
{

	

	/*if (QuestData && Character->GetCurrentQuestNumber() < QuestData->QuestVector.Num())
	{
		
		FQuest LevelQuest = QuestData->QuestVector[Character->GetCurrentQuestNumber()];
			switch (LevelQuest.Objectivetype)
			{
			case EObjectiveType::CollectItem:
			{
				
				int PlayerProgress = Character->GetCollectedItems();
				if (PlayerProgress <= LevelQuest.NumberOfObjectives)
				{
					FText text = FText::FromString(LevelQuest.QuestText);
					mission->QuestText->SetText(text);
					FString FractionText = FString::Printf(TEXT("%d/%d"), PlayerProgress, LevelQuest.NumberOfObjectives);
					mission->Progression->SetText(FText::FromString(FractionText));
				}

				if (PlayerProgress >= LevelQuest.NumberOfObjectives)
				{
					if ((Character->GetCurrentQuestNumber() + 1) <= QuestData->QuestVector.Num())
					{
						Character->AddToQuestNumber(1);
						Character->SetCollectedItems(0);
					}
				}
				break;
			}
			case EObjectiveType::CompleteInteraction:
			{
				int PlayerProgress = Character->GetCollectedItems();
				if (PlayerProgress <= LevelQuest.NumberOfObjectives)
				{
					FText text = FText::FromString(LevelQuest.QuestText);
					mission->QuestText->SetText(text);
					FString FractionText = FString::Printf(TEXT("%d/%d"), PlayerProgress, LevelQuest.NumberOfObjectives);
					mission->Progression->SetText(FText::FromString(FractionText));
				}

				if (PlayerProgress >= LevelQuest.NumberOfObjectives)
				{
					if ((Character->GetCurrentQuestNumber() + 1) <= QuestData->QuestVector.Num())
					{
						Character->AddToQuestNumber(1);
						Character->SetCollectedItems(0);
					}
				}
				break;
			}
			

			default:
				break;
			}
	}*/
}