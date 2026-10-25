using UnrealBuildTool;

public class TaskFlowGraphEditor : ModuleRules
{
    public TaskFlowGraphEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "UnrealEd",
                "GraphEditor",
                "AssetDefinition",
                "Slate",
                "SlateCore",
                "PropertyEditor"
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "TaskFlowGraph"
            });
    }
}
