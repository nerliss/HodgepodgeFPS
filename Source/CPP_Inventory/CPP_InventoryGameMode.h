// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CPP_InventoryGameMode.generated.h"

UCLASS(minimalapi)
class ACPP_InventoryGameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

public:
	ACPP_InventoryGameMode();

	// Hud states
	enum EHUDState : uint8
	{
		HS_Ingame,
		HS_Inventory,
		HS_Shop_General,
		HS_Shop_Lol // placeholder
	};

	// Checks a hud state and then calls ApplyHUD to apply any hud needed
	void ApplyHUDChanges();

	// Getter function for hudstate
	uint8 GetHUDState();

	// Setter function for hudstate, applies a new state and then calls ApplyHUDChanges()
	UFUNCTION(BlueprintCallable, Category = "HUD Function")
	void ChangeHUDState(uint8 NewState);

	// Applies a hud to screen, returns true if successful
	bool ApplyHUD(TSubclassOf<class UUserWidget> WidgetToApply, bool bShowMouseCursor, bool EnableClickEvents);

	// Updated a player's health bar 
// 	UFUNCTION()
// 	void UpdateHealth(class UUserWidget* CurrentWidgetReference, float HP, float MaxHP);

	// placeholder
	bool b_isWindowOpen = false;

protected:

	uint8 HUDState;

	// A hud to be shown ingame
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUDWidgets", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> IngameHUDClass;

	// A hud to be shown in the inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUDWidgets", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> InventoryHUDClass;

	// A hud to be shown in a shop
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUDWidgets", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> ShopGeneralHUDClass;

	UPROPERTY()
	class UUserWidget* CurrentWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UUserWidget* CurrentWidgetReference;

};



