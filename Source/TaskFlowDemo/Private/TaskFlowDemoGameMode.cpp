// Copyright Epic Games, Inc. All Rights Reserved.

#include "TaskFlowDemoGameMode.h"

#include "TaskFlowSystem.h"

ATaskFlowDemoGameMode::ATaskFlowDemoGameMode()
{
	// stub
}

void ATaskFlowDemoGameMode::BeginPlay()
{
	Super::BeginPlay();

	// [EN] Demo of TaskFlowGraph plugin usage: start the configured default task flow.
	// [CN] TaskFlowGraph 插件用法示例：启动配置的默认任务流程
	if (DefaultTaskFlow != nullptr)
	{
		if (UTaskFlowSystem* System = GetWorld()->GetSubsystem<UTaskFlowSystem>())
		{
			System->RunTaskFlow(DefaultTaskFlow, nullptr);
		}
	}
}
