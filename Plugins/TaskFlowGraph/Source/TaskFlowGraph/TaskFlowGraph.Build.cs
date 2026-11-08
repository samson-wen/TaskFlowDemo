using UnrealBuildTool;

public class TaskFlowGraph : ModuleRules
{
    public TaskFlowGraph(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
            });

        // FConnectionDrawingPolicy (used for the animated runtime connection lines) lives in the
        // editor-only GraphEditor module, so depend on it only in editor builds to keep packaging clean.
        if (Target.bBuildEditor)
        {
            // The animated connection-drawing policy uses Slate rendering APIs
            // (FSlateDrawElement / GWhiteBrush) and the editor GraphEditor module.
            PrivateDependencyModuleNames.Add("SlateCore");
            PrivateDependencyModuleNames.Add("Slate");
            PrivateDependencyModuleNames.Add("GraphEditor");
            // UToolMenu / FToolMenuSection for the node right-click menu (editor builds only).
            PrivateDependencyModuleNames.Add("ToolMenus");
        }
    }
}
