// Copyright Epic Games, Inc. All Rights Reserved.

#include "CPP_InventoryCharacter.h"
#include "CPP_InventoryProjectile.h"
#include "CPP_InventoryGameMode.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ACPP_InventoryCharacter

ACPP_InventoryCharacter::ACPP_InventoryCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Range of linetrace
	TraceRange = 250.f; 
}

void ACPP_InventoryCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Inventory default size
	Inventory.SetNum(4);

	// Clearing the current interactable item
	CurrentInteractable = nullptr;
}

void ACPP_InventoryCharacter::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);

	// Initializing linetrace for interactables 
	CheckForInteractables();

	// Initializing linetrace for wall running
	CheckForWall();

}

//////////////////////////////////////////////////////////////////////////
// Input

void ACPP_InventoryCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind interaction events
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACPP_InventoryCharacter::Interact);

	// Bind inventory events
	PlayerInputComponent->BindAction("ToggleInventory", IE_Pressed, this, &ACPP_InventoryCharacter::ToggleInventory);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACPP_InventoryCharacter::OnFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ACPP_InventoryCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACPP_InventoryCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

//////////////////////////////////////////////////////////////////////////
// Movement

void ACPP_InventoryCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			// Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// Spawn the projectile at the muzzle
			World->SpawnActor<ACPP_InventoryProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// Try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// Try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ACPP_InventoryCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ACPP_InventoryCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ACPP_InventoryCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACPP_InventoryCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//////////////////////////////////////////////////////////////////////////
// Inventory

bool ACPP_InventoryCharacter::AddItemToInventory(APickup* Item)
{
	if (Item != NULL)
	{
		const int32 AvaibleSlot = Inventory.Find(nullptr); // finds first slot with a nullptr in it
		if (AvaibleSlot != INDEX_NONE)
		{
			Inventory[AvaibleSlot] = Item;
			return true;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("You cant carry any more items"));
			return false;
		}
	}
	else return false;
}

UTexture2D* ACPP_InventoryCharacter::GetThumbnailAtInventorySlot(int32 Slot)
{
	if (Inventory[Slot] != NULL)
	{
		return Inventory[Slot]->PickupThumbnail;
	}
	else return false;
}

FString ACPP_InventoryCharacter::GetItemNameAtInventorySlot(int32 Slot)
{
	if (Inventory[Slot] != NULL)
	{
		return Inventory[Slot]->ItemName;
	}
	return FString("None");
}

void ACPP_InventoryCharacter::UseItemAtInventorySlot(int32 Slot)
{

	if (Inventory[Slot] != NULL)
	{
		Inventory[Slot]->Use_Implementation();
		Inventory[Slot] = NULL; // delete the item from inventory once used
	}

}

void ACPP_InventoryCharacter::ToggleInventory()
{
	/* Check player's hud state, if inventory is open - close it and otherwise*/
	ACPP_InventoryGameMode* GameMode = Cast<ACPP_InventoryGameMode>(GetWorld()->GetAuthGameMode());

	if (GameMode->GetHUDState() == GameMode->HS_Ingame)
	{
		GameMode->ChangeHUDState(GameMode->HS_Inventory);
	}
	else
	{
		GameMode->ChangeHUDState(GameMode->HS_Ingame);
	}

}

void ACPP_InventoryCharacter::Interact()
{
	if (CurrentInteractable != nullptr)
	{
		CurrentInteractable->Interact_Implementation();
	}
}

void ACPP_InventoryCharacter::CheckForInteractables()
{
	// Raycast start and end traces
	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndTrace = (FirstPersonCameraComponent->GetForwardVector() * TraceRange) + StartTrace;
	
	// Hit result which stores the raycast hit
	FHitResult HitResult;

	// Collision query params - ignore the actor
	FCollisionQueryParams CQP; // CQP - Collision Query Params
	CQP.AddIgnoredActor(this);

	// Cast the linetrace
	GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldDynamic, CQP);

	AInteractable* PotentialInteractable = Cast<AInteractable>(HitResult.GetActor());

	if (PotentialInteractable == NULL)
	{
		HelpText = FString("");
		CurrentInteractable = nullptr;
		return;
	}
	else
	{
		CurrentInteractable = PotentialInteractable;
		HelpText = PotentialInteractable->InteractableHelpText;
	}
}

void ACPP_InventoryCharacter::UpdateMoney(int32 Amount)
{
	Money += Amount;
}

//////////////////////////////////////////////////////////////////////////
// Wall running

void ACPP_InventoryCharacter::CheckForWall()
{
	// Location and rotation of traces
	FVector Location;
	FRotator Rotation;

	// Get location and rotation from player's view point
	GetController()->GetPlayerViewPoint(Location, Rotation);

	// Traces
	FVector StartTrace = Location;
	FVector EndTrace = StartTrace + (Rotation.Vector() * 120);
	
	// Hit result
	FHitResult HitResult;

	// Collision query parameters
	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(this);
	
	// Boolean hit result of line trace
	bool bHitResult = GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, CQP);

	// Debug
	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, -1.f, 0, 2.f);

	// Check whether an actor(wall) has a tag
	// It seems one always needs to check hit result, otherwise the engine crashes
	if (bHitResult)
	{
		// Used for convenience
		AActor* Target = HitResult.GetActor();

		if (Target->ActorHasTag("Climbable"))
		{
			// Do stuff
			InputComponent->BindAction("VerticalWallRun", IE_Pressed, this, &ACPP_InventoryCharacter::VerticalWallRun);
			// VerticalWallRun();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Climbable wall")); // debug
		}
		else
		{
			InputComponent->RemoveActionBinding(14);
		}
	}
}

// Vertical wall run function
void ACPP_InventoryCharacter::VerticalWallRun()
{
	LaunchCharacter(FVector (0, 0, 900), true, true);
}
