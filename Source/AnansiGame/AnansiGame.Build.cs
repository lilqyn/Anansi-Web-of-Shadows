// Copyright 2026 Anansi: Web of Shadows. All Rights Reserved.

using UnrealBuildTool;

public class AnansiGame : ModuleRules
{
	public AnansiGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Ensure subdirectory includes like "Combat/DamageTypes.h" resolve correctly.
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"NavigationSystem",
			"Niagara",
			"UMG",
			"Slate",
			"SlateCore",
			"PhysicsCore",
			"Json",
			"JsonUtilities",
			"AssetRegistry"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AnimGraphRuntime"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"EditorSubsystem",
				"AssetTools"
			});
		}
	}
}
