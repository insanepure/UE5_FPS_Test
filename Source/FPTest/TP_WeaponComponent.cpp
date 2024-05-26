// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "FPTestCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}


void UTP_WeaponComponent::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}
	UWorld* const World = GetWorld();

	if (!World)
	{
		return;
	}


	// This is the same logic as the Projectile firing
	// we want to keep it as before and just transition to the line trace
	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();

	// For Forward Vector, we use the same logic as before, offsetting by MuzzleOffset
	const FVector ForwardVector = SpawnRotation.RotateVector(MuzzleOffset);
	const FVector StartLocation = GetOwner()->GetActorLocation() + ForwardVector;

	// So for line tracing, we also need the forward Vector as well as the End Location
	const FVector EndLocation = ((ForwardVector * HitCastMaxDistance) + StartLocation);

	// Visualize the Trace
	DrawDebugLine(World, StartLocation, EndLocation, FColor::Green, false, 1, 0, 1);

	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;
	if (GWorld->LineTraceSingleByProfile(OutHit, StartLocation, EndLocation, "Projectile", CollisionParams))
	{
		if (OutHit.bBlockingHit && OutHit.Component.IsValid() && OutHit.Component->IsSimulatingPhysics())
		{
			// Just to keep the same behavior as before, we keep the Impulse
			// Normally we would also deal damage, spawn a decal or do something else here
			// but we keep it as that for now
			OutHit.Component->AddImpulseAtLocation(-OutHit.ImpactNormal * HitImpulse, OutHit.Location);
			// Also visualize
			DrawDebugSphere(World, OutHit.Location, 10.0f, 32, FColor::Red, false, 1, 0, 1);
		}
	}
	
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

void UTP_WeaponComponent::AttachWeapon(AFPTestCharacter* TargetCharacter)
{
	Character = TargetCharacter;
	if (Character == nullptr)
	{
		return;
	}

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
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Fire);
		}
	}
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