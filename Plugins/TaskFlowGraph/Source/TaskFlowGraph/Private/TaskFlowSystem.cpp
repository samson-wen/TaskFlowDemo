#include "TaskFlowSystem.h"

#include "GameFramework/Actor.h"
#include "TaskFlowAsset.h"
#include "TaskFlowEvent.h"
#include "TaskFlowRunner.h"

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

	for (UTaskFlowRunner* Runner : ActiveRunners)
	{
		if (Runner != nullptr)
		{
			Runner->DispatchEvent(Event);
		}
	}
}

void UTaskFlowSystem::BroadcastEventToRunner(UTaskFlowRunner* Runner, FName EventName, UObject* Sender, const FString& Payload)
{
	if (Runner == nullptr)
	{
		return;
	}

	FTaskFlowEvent Event;
	Event.EventName = EventName;
	Event.Sender = Sender;
	Event.Payload = Payload;

	Runner->DispatchEvent(Event);
}

void UTaskFlowSystem::BroadcastEventToContext(AActor* Context, FName EventName, UObject* Sender, const FString& Payload)
{
	if (Context == nullptr)
	{
		return;
	}

	FTaskFlowEvent Event;
	Event.EventName = EventName;
	Event.Sender = Sender;
	Event.Payload = Payload;

	for (UTaskFlowRunner* Runner : ActiveRunners)
	{
		if (Runner != nullptr && Runner->GetContext() == Context)
		{
			Runner->DispatchEvent(Event);
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

void UTaskFlowSystem::RegisterExternalReceiver(TScriptInterface<ITaskFlowEventReceiver> Receiver)
{
	UObject* Obj = Receiver.GetObject();
	if (Obj == nullptr)
	{
		return;
	}

	TWeakObjectPtr<UObject> Weak(Obj);
	if (!ExternalReceivers.Contains(Weak))
	{
		ExternalReceivers.Add(Weak);
	}
}

void UTaskFlowSystem::UnregisterExternalReceiver(TScriptInterface<ITaskFlowEventReceiver> Receiver)
{
	if (Receiver.GetObject() != nullptr)
	{
		ExternalReceivers.Remove(TWeakObjectPtr<UObject>(Receiver.GetObject()));
	}
}

void UTaskFlowSystem::BroadcastEventToExternal(const FTaskFlowEvent& Event)
{
	// [EN] 1) Notify any external code bound to the global delegate (the main "outside world" entry point).
	// [CN] 1) 通知绑定到全局委托的外部代码（最主要的“外界”入口）
	OnTaskFlowEvent.Broadcast(Event);

	// [EN] 2) Deliver to externally registered receivers (objects implementing the interface).
	// [CN] 2) 投递给已注册的外界接收者（实现了接口的对象）
	for (const TWeakObjectPtr<UObject>& WeakObj : ExternalReceivers)
	{
		if (UObject* Obj = WeakObj.Get())
		{
			if (Obj->Implements<UTaskFlowEventReceiver>())
			{
				ITaskFlowEventReceiver::Execute_ReceiveTaskFlowEvent(Obj, Event);
			}
		}
	}

	// [EN] Compact dead weak references (objects destroyed without unregistering).
	// [CN] 清理已失效的弱引用（对象被销毁但未取消注册）
	ExternalReceivers.RemoveAll([](const TWeakObjectPtr<UObject>& Weak) { return !Weak.IsValid(); });
}
