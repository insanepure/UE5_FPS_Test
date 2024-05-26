// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FPTestCharacter.h"
#include "TP_WeaponSpawnerComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FPTEST_API UTP_WeaponSpawnerComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	UTP_WeaponSpawnerComponent();
public:
	/** Weapon class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<class AActor> WeaponClass; // Weapon is not reflected in C++ and there is no big reason to do so now, so we keep it as a generic actor

	/** Time until a Weapon Respawns */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	float WeaponRespawnTimerInSeconds = 5.0f;

protected:

	/** Called when the game starts */
	virtual void BeginPlay() override;

	/** Code to spawn the Weapon */
	UFUNCTION(Server, reliable)
	void Server_SpawnWeapon();

	/** Callback for when the weapon is picked up */
	UFUNCTION()
	void OnPickUp(AFPTestCharacter* Character);
};