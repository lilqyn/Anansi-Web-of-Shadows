// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AnansiGameTarget : TargetRules
{
	public AnansiGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.AddRange(new string[] { "AnansiGame" });
	}
}
