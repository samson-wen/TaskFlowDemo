#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Toolkits/IToolkitHost.h"

class UTaskFlowAsset;

class FTaskFlowGraphEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** 打开一个任务流程资产的编辑器 */
    void CreateTaskFlowGraphEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UTaskFlowAsset* InAsset);
};
