#include "TaskFlowGraphEditorModule.h"

#include "EdGraphUtilities.h"
#include "TaskFlowAssetFactory.h"
#include "TaskFlowGraphEditorToolkit.h"
#include "TaskFlowGraphNodeFactory.h"

// Editor module implementation for the TaskFlowGraph plugin.
IMPLEMENT_MODULE(FTaskFlowGraphEditorModule, TaskFlowGraphEditor)

namespace
{
	TSharedPtr<FGraphPanelNodeFactory> TaskFlowGateNodeFactory;
}

void FTaskFlowGraphEditorModule::StartupModule()
{
	TaskFlowGateNodeFactory = MakeShared<FTaskFlowGraphNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(TaskFlowGateNodeFactory);
}

void FTaskFlowGraphEditorModule::ShutdownModule()
{
	if (TaskFlowGateNodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(TaskFlowGateNodeFactory);
		TaskFlowGateNodeFactory.Reset();
	}
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
