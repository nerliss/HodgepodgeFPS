// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup_Statue.h"

APickup_Statue::APickup_Statue()
{

}

void APickup_Statue::Use_Implementation()
{

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Lol kek cheburek"));

}
