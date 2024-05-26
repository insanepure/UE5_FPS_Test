// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class FPTestEditorTarget : TargetRules
{
	public FPTestEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

        // Having this set to something else may cause recompilation when working with different projects and switching between those
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        ExtraModuleNames.Add("FPTest");
	}
}
