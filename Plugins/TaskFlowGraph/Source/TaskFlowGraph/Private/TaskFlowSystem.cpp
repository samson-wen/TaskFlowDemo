#include "TaskFlowSystem.h"

#include "GameFramework/Actor.h"
#include "TaskFlowAsset.h"
#include "TaskFlowEvent.h"
#include "TaskFlowRunner.h"

UTaskFlowSystem* UTaskFlowSystem::Get(const UObject* WorldContext)
{
	UWorld* World = IsValid(WorldContext) ? WorldContext->GetWorld() : nullptr;
	if (World != nullptr)
	{
		return World->GetSubsystem<UTaskFlowSystem>();
	}
	return nullptr;
}

void UTaskFlowSystem::Deinitialize()
{
	// [EN] Before the world is destroyed, stop all running task flows.
	// [CN] 世界销毁前，停止所有正在执行的任务流程
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

UTaskFlowRunner* UTaskFlowSystem::RunTaskFlow(UTaskFlowAsset* InAsset, AActor* InContext)
{
	if (InAsset == nullptr)
	{
		return nullptr;
	}

	// [EN] Use the subsystem as the Outer so the Runner's GetWorld() resolves to the owning World.
	// [CN] 以子系统为 Outer，Runner 的 GetWorld() 即可解析到所在 World
	UTaskFlowRunner* Runner = NewObject<UTaskFlowRunner>(this);
	Runner->AddToRoot();
	Runner->StepDelaySeconds = StepDelaySeconds;
	Runner->OnFinished.AddDynamic(this, &UTaskFlowSystem::HandleTaskFlowFinished);

	ActiveRunners.Add(Runner);

	Runner->Start(InAsset, InContext);

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

UTaskFlowNode* UTaskFlowSystem::FindNodeByName(UTaskFlowAsset* InAsset, FName NodeName) const
{
	if (InAsset == nullptr || NodeName.IsNone())
	{
		return nullptr;
	}

	TArray<UTaskFlowNode*> AllNodes;
	InAsset->GetAllNodes(AllNodes);
	for (UTaskFlowNode* Node : AllNodes)
	{
		if (Node != nullptr && Node->GetNodeName() == NodeName)
		{
			return Node;
		}
	}

	return nullptr;
}

ETaskFlowNodeState UTaskFlowSystem::GetNodeState(UTaskFlowAsset* InAsset, FName NodeName) const
{
	UTaskFlowNode* Node = FindNodeByName(InAsset, NodeName);
	return Node != nullptr ? Node->GetNodeState() : ETaskFlowNodeState::NotExecuted;
}

void UTaskFlowSystem::BroadcastEvent(FName EventName, UObject* Sender, const FString& Payload)
{
	FTaskFlowEvent Event;
	Event.EventName = EventName;
	Event.Sender = Sender;
	Event.Payload = Payload;

	// [CN] 通过 OnTaskFlowEvent 委托广播，所有订阅者（蓝图 / C++）都能收到；系统不再遍历内部节点。
	OnTaskFlowEvent.Broadcast(Event);
}




void UTaskFlowSystem::BroadcastTaskFlowEvent(const FTaskFlowEvent& Event)
{
	// [CN] 以结构体形式广播：与 BroadcastEvent 一致，通过 OnTaskFlowEvent 委托派发。
	OnTaskFlowEvent.Broadcast(Event);
}


void UTaskFlowSystem::HandleTaskFlowFinished(UTaskFlowRunner* Runner)
{
	if (Runner != nullptr)
	{
		ActiveRunners.Remove(Runner);
	}
}


