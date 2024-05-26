// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_WeaponSpawnerComponent.h"
#include "TP_PickupComponent.h"

UTP_WeaponSpawnerComponent::UTP_WeaponSpawnerComponent()
{
}

void UTP_WeaponSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	Server_SpawnWeapon();
}

void UTP_WeaponSpawnerComponent::Server_SpawnWeapon_Implementation()
{
	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}

	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	// Spawn at the Position of the WeaponSpawnerComponent
	// So with this the Spawner can be offset from the ActorPosition
	const FRotator SpawnRotation = FRotator::ZeroRotator;
	const FVector SpawnLocation = GetComponentLocation();

	// Now spawn it
	AActor* SpawnedWeapon = World->SpawnActor<AActor>(WeaponClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
	if (!SpawnedWeapon)
	{
		return;
	}

	// Check for the Pickup Component
	UTP_PickUpComponent* PickupComponent = SpawnedWeapon->GetComponentByClass<UTP_PickUpComponent>();
	if (!PickupComponent)
	{
		return;
	}

	// Add a Callback so we can spawn another Weapon once picked up
	PickupComponent->OnPickUp.AddDynamic(this, &UTP_WeaponSpawnerComponent::OnPickUp);
}

void UTP_WeaponSpawnerComponent::OnPickUp(AFPTestCharacter* Character)
{
	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}

	// Add a Delegate to spawn the weapon after the delay

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&]
	{
		Server_SpawnWeapon();
	});

	// Add the timer to have the Respawn Delay

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, WeaponRespawnTimerInSeconds, false);
}
