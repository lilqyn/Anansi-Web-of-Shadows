// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AnansiGameEditorTarget : TargetRules
{
	public AnansiGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "AnansiGame" });
	}
}
