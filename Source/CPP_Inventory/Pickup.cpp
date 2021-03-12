// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickup.h"
#include "CPP_InventoryCharacter.h"
#include "Kismet/GameplayStatics.h"

APickup::APickup()
{
	// Set up the mesh, item name, value and help text for the pickup
	InteractableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	InteractableMesh->SetSimulatePhysics(true);

	ItemName = FString("Enter an item name");
	InteractableHelpText = FString("Press E to pick the item up");
	Value = 0;

}

void APickup::BeginPlay()
{
	InteractableHelpText = FString::Printf(TEXT("%s: Press E to pick up."), *ItemName);
}

void APickup::Interact_Implementation()
{

	ACPP_InventoryCharacter* Character = Cast<ACPP_InventoryCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (Character->AddItemToInventory(this))
	{
		OnPickedUp();
	}

}

void APickup::Use_Implementation()
{

	GLog->Log("Use() from base pickup class (You shouldn't be seeing this)");

}

void APickup::OnPickedUp()
{
	InteractableMesh->SetVisibility(false);
	InteractableMesh->SetSimulatePhysics(false);
	InteractableMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
