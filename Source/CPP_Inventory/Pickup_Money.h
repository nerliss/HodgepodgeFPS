// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Pickup_Money.generated.h"


/**
 * 
 */
UCLASS()
class CPP_INVENTORY_API APickup_Money : public APickup
{
	GENERATED_BODY()

	APickup_Money();

	virtual void Interact_Implementation() override;

	virtual void BeginPlay();

};
