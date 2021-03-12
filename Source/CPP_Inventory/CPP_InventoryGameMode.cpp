// Copyright Epic Games, Inc. All Rights Reserved.

#include "CPP_InventoryGameMode.h"
#include "CPP_InventoryHUD.h"
#include "CPP_InventoryCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ProgressBar.h"

void ACPP_InventoryGameMode::BeginPlay()
{
	// As soon as player enters the game apply hud to the screen
	ApplyHUDChanges();
}

ACPP_InventoryGameMode::ACPP_InventoryGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ACPP_InventoryHUD::StaticClass();

	// Set the default hudstate
	HUDState = EHUDState::HS_Ingame;
}

void ACPP_InventoryGameMode::ApplyHUDChanges()
{
	// Remove the previous hud
	if (CurrentWidget != nullptr)
	{
		CurrentWidget->RemoveFromParent();
	}

	switch (HUDState)
	{
	case EHUDState::HS_Ingame:
	{
		ApplyHUD(IngameHUDClass, false, false);
		break;
	}
	case EHUDState::HS_Inventory:
	{
		ApplyHUD(InventoryHUDClass, true, true);
		break;
	}
	case EHUDState::HS_Shop_General:
	{
		ApplyHUD(ShopGeneralHUDClass, true, true);
		break;
	}
	default:
	{
		ApplyHUD(IngameHUDClass, false, false);
		break;
	}
	}

}

uint8 ACPP_InventoryGameMode::GetHUDState()
{
	return HUDState;
}

void ACPP_InventoryGameMode::ChangeHUDState(uint8 NewState)
{
	HUDState = NewState;
	ApplyHUDChanges();
}

bool ACPP_InventoryGameMode::ApplyHUD(TSubclassOf<class UUserWidget> WidgetToApply, bool bShowMouseCursor, bool EnableClickEvents)
{
	// Get the reference to the player and the controller
	ACPP_InventoryCharacter* MyCharacter = Cast<ACPP_InventoryCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	APlayerController* MyController = GetWorld()->GetFirstPlayerController();

	// Nullcheck whatever widget we're trying to apply
	if (WidgetToApply != nullptr)
	{
		// Set mouse visibility and click events according to params
		MyController->bShowMouseCursor = bShowMouseCursor;
		MyController->bEnableClickEvents = EnableClickEvents;

		// Create the widget
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetToApply);

		// Current widget reference (for UpdateHealth())
		CurrentWidgetReference = CurrentWidget;

		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
			return true;
		}
		else return false;
	}
	else return false;
}

// void ACPP_InventoryGameMode::UpdateHealth(class UUserWidget* CurrentWidgetReference, float HP, float MaxHP)
// {
// 	CurrentWidgetReference = UProgressBar::SetPercent((HP / MaxHP));
// }

