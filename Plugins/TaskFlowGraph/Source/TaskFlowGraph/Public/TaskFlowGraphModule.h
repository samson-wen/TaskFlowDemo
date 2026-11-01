#pragma once

#include "Modules/ModuleManager.h"

/** [EN] Runtime module for the TaskFlowGraph plugin. [CN] TaskFlowGraph 插件的运行时模块。 */
class FTaskFlowGraphModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
