// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "FPTestCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
	// Set the Ammunition to be the Magazine Value
	CurrentAmmunition = MaxMagazine;
}


void UTP_WeaponComponent::FireSingle()
{
	if (ShootType != EWeaponShootType::Single)
		return;

	Fire_Internal(1.0f, 2);
}

void UTP_WeaponComponent::FireAutomatic()
{
	if (ShootType != EWeaponShootType::Automatic || !CanShoot)
		return;

	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}

	CanShoot = false;

	// Add a Delegate to spawn the weapon after the delay

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&]
	{
		CanShoot = true;
	});

	// Add the timer to have the Respawn Delay

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, AutomaticCooldown, false);

	Fire_Internal(0.5f, 1);
}


void UTP_WeaponComponent::FireCharged()
{
	if (ShootType != EWeaponShootType::Charged)
		return;

	Fire_Internal(5.0f, 4);
}

void UTP_WeaponComponent::StartFireCharged()
{
	if (ShootType != EWeaponShootType::Charged)
		return;

	All_StartCharging();
}


void UTP_WeaponComponent::Fire_Internal(float ImpactModifier, int32 Damage)
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Fire"));

	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}
	UWorld* const World = GetWorld();

	if (!World)
	{
		return;
	}

	// if Ammunition reaches 0, we play the NoAmmo Sound
	// It would be extremely weird if the Ammunition reaches below zero
	// But as other people might also work with this and a value like this could be manipulated throughout the code (Which is bad tbh, but hard to manage when working with a big team)
	// we just catch also cases below 0 and reset it, just to be safe
	if (CurrentAmmunition <= 0)
	{
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No Ammo"));
		CurrentAmmunition = 0;
		// we have got no ammo, so just play a sound and skip firing
		if (NoAmmoSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, NoAmmoSound, Character->GetActorLocation());
		}
		return;
	}

	// Reduce ammunition by one
	CurrentAmmunition--;

	OnAmmoChanged.Broadcast(CurrentAmmunition);

	// This is the same logic as the Projectile firing
	// we want to keep it as before and just transition to the line trace
	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();

	// For Forward Vector, we use the same logic as before, offsetting by MuzzleOffset
	const FVector ForwardVector = SpawnRotation.RotateVector(MuzzleOffset);
	const FVector StartLocation = GetOwner()->GetActorLocation() + ForwardVector;

	// So for line tracing, we also need the forward Vector as well as the End Location
	const FVector EndLocation = ((ForwardVector * HitCastMaxDistance) + StartLocation);

	Server_FireTrace(StartLocation, EndLocation, ImpactModifier, Damage);
}


void UTP_WeaponComponent::Server_FireTrace_Implementation(FVector StartLocation, FVector EndLocation, float ImpactModifier, int32 Damage)
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Server_FireTrace"));

	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}

	// Do the line Trace going from the Start to the End
	// This should mirror the behavior of the projectile before, but only as trace

	All_FireVisual(StartLocation, EndLocation);

	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;
	if (World->LineTraceSingleByChannel(OutHit, StartLocation, EndLocation, COLLISION_SHOTTRACE, CollisionParams))
	{
		if (!OutHit.bBlockingHit || !OutHit.Component.IsValid())
			return;

		AFPTestCharacter* character = Cast< AFPTestCharacter>(OutHit.GetActor());
		if (character)
		{
			character->Server_OnDamageTaken(Damage);
		}
		else if (OutHit.Component->IsSimulatingPhysics())
		{
			// Just to keep the same behavior as before, we keep the Impulse
			// Normally we would also deal damage, spawn a decal or do something else here
			// but we keep it as that for now
			OutHit.Component->AddImpulseAtLocation(-OutHit.ImpactNormal * HitImpulse * ImpactModifier, OutHit.Location);
		}
		// Also visualize
		DrawDebugSphere(World, OutHit.Location, 10.0f, 32, FColor::Red, false, 1, 0, 1);
	}
}

void UTP_WeaponComponent::All_FireVisual_Implementation(FVector StartLocation, FVector EndLocation)
{
	UWorld* const World = GetWorld();
	if (!World || !Character)
	{
		return;
	}

	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("All_FireVisual"));

	// Visualize the Trace
	DrawDebugLine(World, StartLocation, EndLocation, FColor::Green, false, 1, 0, 1);

	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}

	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void UTP_WeaponComponent::Reload()
{
	// Normally this would trigger an animation which disables firing
	// There was nothing like this written in the document, so I keep it simple
	CurrentAmmunition = MaxMagazine;

	OnAmmoChanged.Broadcast(CurrentAmmunition);

	All_Reload();
}

void UTP_WeaponComponent::ToggleType()
{
	switch (ShootType)
	{
	case EWeaponShootType::Single:
		ShootType = EWeaponShootType::Automatic;
		break;
	case EWeaponShootType::Automatic:
		ShootType = EWeaponShootType::Charged;
		break;
	case EWeaponShootType::Charged:
		ShootType = EWeaponShootType::Single;
		break;
	}
}


void UTP_WeaponComponent::All_Reload_Implementation()
{
	if (ReloadSound != nullptr && Character)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, Character->GetActorLocation());
	}
}


void UTP_WeaponComponent::All_StartCharging_Implementation()
{
	if (ChargeSound != nullptr && Character)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ChargeSound, Character->GetActorLocation());
	}
}

void UTP_WeaponComponent::AttachWeapon(AFPTestCharacter* TargetCharacter)
{
	if (TargetCharacter == nullptr)
	{
		return;
	}

	// Set the owner of the Weapon to the Character
	// This allows us to receive Input as the client is now the owner
	// But this would lead in PC games to cheating, where it would be possible to manipulate values
	// like the magazine, which is now handled by the weapon
	// We can leave it as it is as it was not a requirement
	// but I still want to point out the issue
	GetOwner()->SetOwner(TargetCharacter);

	// If the Character already has a weapon, we destroy ourself
	if (TargetCharacter->GetHasRifle())
	{
		DestroySelf();
		return;
	}

	Character = TargetCharacter;

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));
	
	// switch bHasRifle so the animation blueprint can switch to another animation set
	Character->SetHasRifle(true);

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireSingleAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::FireSingle);
			EnhancedInputComponent->BindAction(FireAutomaticAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::FireAutomatic);
			EnhancedInputComponent->BindAction(FireChargedAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::FireCharged);
			EnhancedInputComponent->BindAction(FireChargedAction, ETriggerEvent::Started, this, &UTP_WeaponComponent::StartFireCharged);
			// Reload
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Reload);
			// Toggle Type
			EnhancedInputComponent->BindAction(ToggleTypeAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::ToggleType);
		}
	}
}

void UTP_WeaponComponent::DestroySelf()
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Destroy Weapon"));
	GetOwner()->Destroy();
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character == nullptr)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}

void UTP_WeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Call the Super
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Add properties to replicated for the derived class
	DOREPLIFETIME(UTP_WeaponComponent, Character);
}