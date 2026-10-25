#include "TaskFlowGraphEditorModule.h"

#include "TaskFlowAssetFactory.h"
#include "TaskFlowGraphEditorToolkit.h"

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
