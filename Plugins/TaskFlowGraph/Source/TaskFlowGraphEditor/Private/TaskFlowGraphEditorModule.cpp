#include "TaskFlowGraphEditorModule.h"

#include "TaskFlowAssetFactory.h"
#include "TaskFlowGraphEditorToolkit.h"

// [EN] Editor module implementation for the TaskFlowGraph plugin.
// [CN] TaskFlowGraph 插件的编辑器模块实现。
IMPLEMENT_MODULE(FTaskFlowGraphEditorModule, TaskFlowGraphEditor)

void FTaskFlowGraphEditorModule::StartupModule()
{
}

void FTaskFlowGraphEditorModule::ShutdownModule()
{
}

void FTaskFlowGraphEditorModule::CreateTaskFlowGraphEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UTaskFlowAsset* InAsset)
{
    if (InAsset == nullptr)
    {
        return;
    }

    TSharedRef<FTaskFlowGraphEditor> NewEditor = MakeShared<FTaskFlowGraphEditor>();
    NewEditor->InitTaskFlowGraphEditor(Mode, InitToolkitHost, InAsset);
}
