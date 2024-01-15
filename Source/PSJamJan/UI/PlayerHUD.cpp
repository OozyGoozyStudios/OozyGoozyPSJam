// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"
#include "UI.h"


APlayerHUD::APlayerHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> UserInterfaceBPClass(TEXT("/Game/FirstPerson/Blueprints/UserInterface/WBP_UI"));
	if (!ensure(UserInterfaceBPClass.Class != nullptr)) return;

	UserInterfaceClass = UserInterfaceBPClass.Class;

}



void APlayerHUD::BeginPlay()
{

	Super::BeginPlay();
	
	
	if (!ensure(UserInterfaceClass != nullptr)) return;
	UserInterface = CreateWidget<UUI>(GetWorld(), UserInterfaceClass);
	if (!ensure(UserInterface != nullptr)) return;

	UserInterface->Setup();
	UserInterface->Mission();
	


}

void APlayerHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (UserInterface)
	{
		UserInterface->UpdateText();
	}
}