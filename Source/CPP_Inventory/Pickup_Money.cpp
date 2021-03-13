// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup_Money.h"
#include "CPP_InventoryCharacter.h"
#include "Kismet/GameplayStatics.h"

APickup_Money::APickup_Money()
{
	Value = 0;
}

void APickup_Money::BeginPlay()
{
	Super::BeginPlay();
}

void APickup_Money::Interact_Implementation()
{
	
	ACPP_InventoryCharacter* Character = Cast<ACPP_InventoryCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	Character->UpdateMoney(Value);

	Destroy();

}
