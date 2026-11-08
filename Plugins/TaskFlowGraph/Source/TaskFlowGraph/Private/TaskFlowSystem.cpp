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
	// Before the world is destroyed, stop all running task flows.
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

	// Use the subsystem as the Outer so the Runner's GetWorld() resolves to the owning World.
	UTaskFlowRunner* Runner = NewObject<UTaskFlowRunner>(this);
	Runner->AddToRoot();
	Runner->StepDelaySeconds = StepDelaySeconds;
	Runner->OnFinished.AddDynamic(this, &UTaskFlowSystem::HandleTaskFlowFinished);

	ActiveRunners.Add(Runner);

	Runner->Start(InAsset, InContext);

	return Runner;
}

bool UTaskFlowSystem::IsTaskFlowAssetRunning(const UTaskFlowAsset* InAsset) const
{
	if (InAsset == nullptr)
	{
		return false;
	}

	for (const TObjectPtr<UTaskFlowRunner>& RunnerPtr : ActiveRunners)
	{
		const UTaskFlowRunner* Runner = RunnerPtr.Get();
		if (Runner != nullptr && Runner->IsRunning() && Runner->Asset.Get() == InAsset)
		{
			return true;
		}
	}

	return false;
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

	// Broadcast through OnTaskFlowEvent so all subscribers (Blueprint / C++) receive it; the system no longer traverses internal nodes.
	OnTaskFlowEvent.Broadcast(Event);
}




void UTaskFlowSystem::BroadcastTaskFlowEvent(const FTaskFlowEvent& Event)
{
	// Broadcast a structured event: like BroadcastEvent, it is dispatched through OnTaskFlowEvent.
	OnTaskFlowEvent.Broadcast(Event);
}


void UTaskFlowSystem::HandleTaskFlowFinished(UTaskFlowRunner* Runner)
{
	if (Runner != nullptr)
	{
		ActiveRunners.Remove(Runner);
	}
}


