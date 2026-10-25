#include "TaskFlowSystem.h"

#include "TaskFlowAsset.h"
#include "TaskFlowRunner.h"

void UTaskFlowSystem::Deinitialize()
{
	// 世界销毁前，停止所有正在执行的任务流程
	TArray<TObjectPtr<UTaskFlowRunner>> Runners = MoveTemp(ActiveRunners);
	for (UTaskFlowRunner* Runner : Runners)
	{
		if (Runner != nullptr)
		{
			Runner->Stop();
		}
	}

	Super::Deinitialize();
}

UTaskFlowRunner* UTaskFlowSystem::RunTaskFlow(UTaskFlowAsset* InAsset)
{
	if (InAsset == nullptr)
	{
		return nullptr;
	}

	// 以子系统为 Outer，Runner 的 GetWorld() 即可解析到所在 World
	UTaskFlowRunner* Runner = NewObject<UTaskFlowRunner>(this);
	Runner->AddToRoot();
	Runner->StepDelaySeconds = StepDelaySeconds;
	Runner->OnFinished.AddDynamic(this, &UTaskFlowSystem::HandleTaskFlowFinished);

	ActiveRunners.Add(Runner);

	Runner->Start(InAsset);

	return Runner;
}

void UTaskFlowSystem::StopTaskFlow(UTaskFlowRunner* InRunner)
{
	if (InRunner != nullptr)
	{
		InRunner->Stop();
	}
}

void UTaskFlowSystem::StopAllTaskFlows()
{
	TArray<TObjectPtr<UTaskFlowRunner>> Runners = MoveTemp(ActiveRunners);
	for (UTaskFlowRunner* Runner : Runners)
	{
		if (Runner != nullptr)
		{
			Runner->Stop();
		}
	}
}

void UTaskFlowSystem::HandleTaskFlowFinished(UTaskFlowRunner* Runner)
{
	if (Runner != nullptr)
	{
		ActiveRunners.Remove(Runner);
	}
}
