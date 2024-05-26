// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "TP_WeaponComponent.generated.h"

class AFPTestCharacter;

// I dislike usage of channels for this in c++, because code-wise, you have no idea if this is really the correct trace channel you want
// you need to manually check the .ini file and check in there
// so if somebody changes the ini, file this might break ...
#define COLLISION_SHOTTRACE		ECC_GameTraceChannel2

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32, NewAmmo);

UENUM(BlueprintType)
enum class EWeaponShootType : uint8
{
	Single,
	Automatic,
	Charged
};

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPTEST_API UTP_WeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	/** Delegate when the Ammo has changed */
	UPROPERTY(BlueprintAssignable, Category = Gameplay)
	FOnAmmoChanged OnAmmoChanged;
	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* FireSound;

	/** Sound to play when we cannot fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* NoAmmoSound;

	/** Sound to play when we are reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* ReloadSound;

	/** Sound to play when we are charging a shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* ChargeSound;
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	/** Fire Single Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireSingleAction;

	/** Fire Automatic Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAutomaticAction;

	/** Fire Charged Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireChargedAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;

	/** Toggle Weapon Type Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ToggleTypeAction;

	/** End Distance of the Raycast for the Hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float HitCastMaxDistance = 1000.0f;

	/** Impulse to apply when hitting something */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float HitImpulse = 100000.0f;

	/** How many ammunition the weapon can hold at Max */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 MaxMagazine = 30;

	/** Cooldown for Automatic Shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float AutomaticCooldown = 0.2f;

	/** Ammunition the weapon holds currently */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
	int CurrentAmmunition = 0;


	/** How many ammunition the weapon can hold at Max */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	EWeaponShootType ShootType = EWeaponShootType::Single;


	/** Replicate the */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	AFPTestCharacter* Character;

	/** Sets default values for this component's properties */
	UTP_WeaponComponent();

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void AttachWeapon(AFPTestCharacter* TargetCharacter);

	/** Fire a single Shot */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FireSingle();

	/** Fire a single Shot */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FireAutomatic();

	/** Fire a charged Shot */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FireCharged();

	/** Start a charged Shot */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFireCharged();

	/** Common function for firing all */
	void Fire_Internal(float ImpactModifier, int32 Damage);

	/** Do the fire logic on the server */
	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Weapon")
	void Server_FireTrace(FVector StartLocation, FVector EndLocation, float ImpactModifier, int32 Damage);

	/* Do the Visual for everyone  */
	UFUNCTION(NetMulticast, reliable, BlueprintCallable, Category = "Weapon")
	void All_FireVisual(FVector StartLocation, FVector EndLocation);

	/** Make the weapon Reload */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();

	/** Change the Type of the Weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ToggleType();

	/** Call of Reload executed for everyone to handle visual or sound */
	UFUNCTION(netMulticast, reliable, BlueprintCallable, Category = "Weapon")
	void All_Reload();

	/** Call of Charge executed for everyone to handle visual or sound */
	UFUNCTION(netMulticast, reliable, BlueprintCallable, Category = "Weapon")
	void All_StartCharging();

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DestroySelf();

protected:
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Override Replicate Properties function
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


private:
	/** Boolean just for the cooldown */
	bool CanShoot = true;
};
