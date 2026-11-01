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

        // [EN] FConnectionDrawingPolicy (used for the animated runtime connection lines) lives in the
        // [CN] FConnectionDrawingPolicy（用于运行时连线动画）属于编辑器模块 GraphEditor，
        // [EN] editor-only GraphEditor module, so depend on it only in editor builds to keep packaging clean.
        // [CN] 因此仅在编辑器构建中加入该依赖，以保证打包干净。
        if (Target.bBuildEditor)
        {
            // [EN] The animated connection-drawing policy uses Slate rendering APIs
            // [CN] 运行时连接线动效的绘制策略用到了 Slate 渲染 API
            // [EN] (FSlateDrawElement / GWhiteBrush) and the editor GraphEditor module.
            // [CN]（FSlateDrawElement / GWhiteBrush）以及编辑器模块 GraphEditor。
            PrivateDependencyModuleNames.Add("SlateCore");
            PrivateDependencyModuleNames.Add("Slate");
            PrivateDependencyModuleNames.Add("GraphEditor");
        }
    }
}
