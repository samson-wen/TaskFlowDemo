#pragma once

#include "Modules/ModuleManager.h"

/** Runtime module for the TaskFlowGraph plugin. */
class FTaskFlowGraphModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
