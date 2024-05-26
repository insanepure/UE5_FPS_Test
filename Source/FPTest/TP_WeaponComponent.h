// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "TP_WeaponComponent.generated.h"

class AFPTestCharacter;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPTEST_API UTP_WeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* FireSound;

	/** Sound to play when we cannot fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* NoAmmoSound;

	/** Sound to play when we are reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* ReloadSound;
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;

	/** End Distance of the Raycast for the Hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float HitCastMaxDistance = 1000.0f;

	/** Impulse to apply when hitting something */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float HitImpulse = 100000.0f;

	/** How many ammunition the weapon can hold at Max */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 MaxMagazine = 30;

	/** Sets default values for this component's properties */
	UTP_WeaponComponent();

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void AttachWeapon(AFPTestCharacter* TargetCharacter);

	/** Make the weapon Fire a Projectile */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();

	/** Do the fire logic on the server */
	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Weapon")
	void Server_FireTrace(FVector StartLocation, FVector EndLocation);

	/* Do the Visual for everyone  */
	UFUNCTION(NetMulticast, reliable, BlueprintCallable, Category = "Weapon")
	void All_FireVisual(FVector StartLocation, FVector EndLocation);

	/** Make the weapon Reload */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();

	/** Call of Reload executed for everyone to handle visual or sound */
	UFUNCTION(netMulticast, reliable, BlueprintCallable, Category = "Weapon")
	void All_Reload();

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Weapon")
	void Server_DestroySelf();

protected:
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** The Character holding this weapon*/
	AFPTestCharacter* Character;
	/** Ammunition the weapon holds currently */
	int CurrentAmmunition;
};
