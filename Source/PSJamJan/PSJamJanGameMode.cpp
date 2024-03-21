// Copyright Epic Games, Inc. All Rights Reserved.

#include "PSJamJanGameMode.h"
#include "PSJamJanCharacter.h"
#include "UObject/ConstructorHelpers.h"

void APSJamJanGameMode::BeginPlay()
{
	/**
	Super::BeginPlay();

	FFileHelper::SaveStringToFile(TEXT(RAW_APP_ID), TEXT("steam_appid.txt"));

	SteamAPI_RestartAppIfNecessary(atoi(APP_ID));

	if (SteamAPI_Init())
	{
		MyID = SteamUser()->GetSteamID();
	}
	*/
		
}

APSJamJanGameMode::APSJamJanGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
