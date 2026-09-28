// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SMAGF : ModuleRules
{
	public SMAGF(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"DeveloperSettings",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "SlateCore" });

		PublicIncludePaths.AddRange(new string[] {
			"SMAGF",
			"SMAGF/Variant_Strategy",
			"SMAGF/Variant_Strategy/UI",
			"SMAGF/Variant_TwinStick",
			"SMAGF/Variant_TwinStick/AI",
			"SMAGF/Variant_TwinStick/Gameplay",
			"SMAGF/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
