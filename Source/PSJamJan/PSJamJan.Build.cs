// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class PSJamJan : ModuleRules
{
    public PSJamJan(ReadOnlyTargetRules Target) : base(Target)
    {

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });

        PrivateDependencyModuleNames.AddRange(new string[] { "OnlineSubsystem", "OnlineSubsystemNull", "OnlineSubsystemSteam", "OnlineSubsystemUtils" });

        PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Steam", "lib", "steam_api64.lib"));
    }
}
