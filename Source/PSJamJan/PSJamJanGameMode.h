// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#pragma warning(push)
#pragma warning(disable: 4996)
#include "Steam/steam_api.h"
#pragma warning(pop)

#include "PSJamJanGameMode.generated.h"

#define RAW_APP_ID "2866730"

UCLASS(minimalapi)
class APSJamJanGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	APSJamJanGameMode();

	static constexpr char* APP_ID = RAW_APP_ID;
	CSteamID MyID;
};



