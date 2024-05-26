// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class FPTestTarget : TargetRules
{
	public FPTestTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		// Having this set to something else may cause recompilation when working with different projects and switching between those
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        ExtraModuleNames.Add("FPTest");
	}
}
