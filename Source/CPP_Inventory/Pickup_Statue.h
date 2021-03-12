// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Pickup_Statue.generated.h"

/**
 * 
 */
UCLASS()
class CPP_INVENTORY_API APickup_Statue : public APickup
{
	GENERATED_BODY()
public:

	APickup_Statue();

	virtual void Use_Implementation() override;


};
