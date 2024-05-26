// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPTestGameMode.h"
#include "FPTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPTestGameMode::AFPTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
