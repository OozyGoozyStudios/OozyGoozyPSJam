// Copyright Epic Games, Inc. All Rights Reserved.

#include "PSJamJanGameMode.h"
#include "PSJamJanCharacter.h"
#include "UObject/ConstructorHelpers.h"

APSJamJanGameMode::APSJamJanGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
