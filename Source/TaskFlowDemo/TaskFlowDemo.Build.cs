// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TaskFlowDemo : ModuleRules
{
	public TaskFlowDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"TaskFlowGraph"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Headers now live under Public/; register every subfolder as an include root so
		// existing bare #includes (e.g. "#include \"CombatAttacker.h\"", "TaskFlowVolume.h")
		// keep resolving after the Public/Private split. Enumerated at build-script time so
		// any new subfolder is picked up automatically.
		string TaskFlowPublicDir = System.IO.Path.Combine(ModuleDirectory, "Public");
		PublicIncludePaths.Add(TaskFlowPublicDir);
		foreach (string SubDir in System.IO.Directory.GetDirectories(TaskFlowPublicDir, "*", System.IO.SearchOption.AllDirectories))
		{
			PublicIncludePaths.Add(SubDir);
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
