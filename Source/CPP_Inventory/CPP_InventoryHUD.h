// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CPP_InventoryHUD.generated.h"

UCLASS()
class ACPP_InventoryHUD : public AHUD
{
	GENERATED_BODY()

public:
	ACPP_InventoryHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

