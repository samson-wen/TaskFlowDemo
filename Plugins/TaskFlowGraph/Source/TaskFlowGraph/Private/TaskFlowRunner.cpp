#include "TaskFlowRunner.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Misc/MessageDialog.h"
#include "Nodes/TaskFlowAction.h"
#include "Nodes/TaskFlowNode.h"
#include "Nodes/TaskFlowNode_End.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "Nodes/TaskFlowNode_Start.h"
#include "TaskFlowAsset.h"
#include "TaskFlowEvent.h"
#include "TaskFlowSystem.h"
#include "TaskFlowTypes.h"

DEFINE_LOG_CATEGORY(LogTaskFlow);

namespace
{
	/**
	 * The single formatter for every node lifecycle log line, so Begin and End always share the same layout.
	 * Format: [TaskFlow] Flow=<Asset> | Event=<Begin|End> | Node=<Name> | Class=<Class> | Detail=<...>
	 */
	void LogNodeLifecycle(const UTaskFlowRunner* Runner, const UTaskFlowNode* Node, const TCHAR* Event, const TCHAR* Detail)
	{
		const FString FlowName = (Runner != nullptr && Runner->Asset.Get() != nullptr)
			? Runner->Asset->GetName()
			: FString(TEXT("<None>"));

		const FString ClassName = (Node != nullptr) ? Node->GetClass()->GetName() : FString(TEXT("<None>"));

		FString NodeName = TEXT("<None>");
		if (Node != nullptr)
		{
			const FName LogicalName = Node->GetNodeName();
			NodeName = LogicalName.IsNone() ? Node->GetName() : LogicalName.ToString();
		}

		LogTaskFlowLifecycle(FlowName, Event, NodeName, *ClassName, Detail);
	}
}

UTaskFlowRunner* UTaskFlowRunner::StartTaskFlow(const UObject* WorldContextObject, UTaskFlowAsset* InAsset, AActor* InContext)
{
	if (InAsset == nullptr)
	{
		return nullptr;
	}
	if (WorldContextObject == nullptr)
	{
		return nullptr;
	}

	UTaskFlowRunner* Runner = NewObject<UTaskFlowRunner>();
	Runner->AddToRoot();

	Runner->WorldPtr = WorldContextObject != nullptr ? WorldContextObject->GetWorld() : nullptr;
	Runner->Start(InAsset, InContext);

	return Runner;
}

void UTaskFlowRunner::Start(UTaskFlowAsset* InAsset, AActor* InContext)
{
	if (InAsset == nullptr || bRunning)
	{
		return;
	}

	// Distinguish by the context type passed in: PlayerState = personal task, GameState = global task.
	Context = InContext;
	PlayerStateOwner = Cast<APlayerState>(InContext);
	GameStateOwner = Cast<AGameStateBase>(InContext);

	// Validate that the context matches the asset's task type (editor popup).
	ValidateContextType(InAsset, InContext);

	Asset = InAsset;
	bRunning = true;

	// Initialize the runtime blackboard from the asset blackboard.
	BlackboardValues.Empty();
	if (Asset->Blackboard.Get() != nullptr)
	{
		for (const FTaskFlowBlackboardEntry& Entry : Asset->Blackboard->Entries)
		{
			BlackboardValues.Add(Entry.KeyName, Entry);
		}
	}

	// Reset all node states to NotExecuted.
	TArray<UTaskFlowNode*> AllNodes;
	InAsset->GetAllNodes(AllNodes);
	for (UTaskFlowNode* Node : AllNodes)
	{
		if (Node != nullptr)
		{
			Node->SetNodeState(ETaskFlowNodeState::NotExecuted);
		}
	}

	if (UTaskFlowNode_Start* StartNode = InAsset->GetStartNode())
	{
		PendingNodes.Add(StartNode);
	}

	// The Start node broadcasts the Started event when it executes. Doing it here as well
	// would produce a duplicate event for every flow.
	Advance();
}

void UTaskFlowRunner::Stop()
{
	if (!bRunning)
	{
		return;
	}

	// Keep ActiveNodes so FinishFlow can emit the matching End logs for these aborted nodes.
	PendingNodes.Empty();

	ClearAdvanceTimer();

	// Manual stop: reset all nodes to NotExecuted (gray).
	if (Asset.Get() != nullptr)
	{
		TArray<UTaskFlowNode*> AllNodes;
		Asset->GetAllNodes(AllNodes);
		for (UTaskFlowNode* Node : AllNodes)
		{
			if (Node != nullptr)
			{
				Node->SetNodeState(ETaskFlowNodeState::NotExecuted);
			}
		}
	}

	FinishFlow();
}

bool UTaskFlowRunner::IsRunning() const
{
	return bRunning;
}

UTaskFlowNode* UTaskFlowRunner::GetCurrentNode() const
{
	return ActiveNodes.Num() > 0 ? ActiveNodes[0].Get() : nullptr;
}

void UTaskFlowRunner::GetActiveNodes(TArray<UTaskFlowNode*>& OutNodes) const
{
	OutNodes.Reset();
	OutNodes.Reserve(ActiveNodes.Num());
	for (const TObjectPtr<UTaskFlowNode>& NodePtr : ActiveNodes)
	{
		if (UTaskFlowNode* Node = NodePtr.Get())
		{
			OutNodes.Add(Node);
		}
	}
}

UWorld* UTaskFlowRunner::GetTaskFlowWorld() const
{
	UWorld* World = WorldPtr.Get();
	return World != nullptr ? World : GetWorld();
}

void UTaskFlowRunner::NotifyNodeFinished(UTaskFlowNode* Node)
{
	if (!bRunning || Node == nullptr)
	{
		return;
	}

	// Only accept completion callbacks from the current active branch; late callbacks after the flow has ended are ignored.
	const int32 ActiveIndex = ActiveNodes.IndexOfByPredicate([Node](const TObjectPtr<UTaskFlowNode>& Ptr)
	{
		return Ptr.Get() == Node;
	});
	if (ActiveIndex == INDEX_NONE)
	{
		return;
	}

	ActiveNodes.RemoveAt(ActiveIndex, 1, EAllowShrinking::No);

	// Merge nodes stay Executing (yellow) until their condition is met, e.g. AND waiting for all inputs.
	if (Node->ShouldMarkSucceededOnFinish())
	{
		Node->SetNodeState(ETaskFlowNodeState::Succeeded);
	}
	LogNodeLifecycle(this, Node, TEXT("End"), TEXT("Finished"));

	TArray<UTaskFlowNode*> NextNodes;
	Node->GetNextNodes(NextNodes);
	for (int32 Index = NextNodes.Num() - 1; Index >= 0; --Index)
	{
		if (NextNodes[Index] != nullptr)
		{
			PendingNodes.Insert(NextNodes[Index], 0);
		}
	}

	if (StepDelaySeconds > 0.f)
	{
		ScheduleAdvance();
	}
	else
	{
		Advance();
	}
}

void UTaskFlowRunner::EmitEvent(const FTaskFlowEvent& Event)
{
	// Broadcast through the system's OnTaskFlowEvent delegate so all subscribers (Blueprint / C++) receive it.
	UTaskFlowSystem* System = GetTaskFlowSystem();
	if (System != nullptr)
	{
		System->BroadcastTaskFlowEvent(Event);
	}
}

void UTaskFlowRunner::EmitEventByName(FName EventName, UObject* Sender, const FString& Payload)
{
	FTaskFlowEvent Event;
	Event.EventName = EventName;
	Event.Sender = Sender;
	Event.Payload = Payload;
	EmitEvent(Event);
}

UTaskFlowSystem* UTaskFlowRunner::GetTaskFlowSystem() const
{
	UWorld* World = GetTaskFlowWorld();
	return World != nullptr ? World->GetSubsystem<UTaskFlowSystem>() : nullptr;
}

void UTaskFlowRunner::Advance()
{
	if (bAdvancing)
	{
		// Already advancing (a synchronous node called FinishExecution); let the outer loop continue.
		return;
	}

	bAdvancing = true;

	while (bRunning)
	{
		// When there are no pending nodes: if branches are still active, wait for their completion callbacks; the flow ends only once the active set is empty too.
		if (PendingNodes.Num() == 0)
		{
			if (ActiveNodes.Num() == 0)
			{
				// All branches finished without an explicit End node: still a normal completion.
				EmitEventByName(TaskFlowConstants::EventName_Completed, this, Asset.Get() != nullptr ? Asset->GetName() : FString());
				FinishFlow();
			}
			break;
		}

		// Take all currently pending nodes as one batch and start them together.
		//      Synchronous nodes add their successors to PendingNodes inside BeginExecution; the next loop iteration handles them as a new batch.
		const TArray<TObjectPtr<UTaskFlowNode>> Batch = MoveTemp(PendingNodes);

		// Check the whole batch for End nodes first: the presence of an End node ends the entire flow.
		//      This check runs before any node is started, so the End node's position within the batch does not matter.
		bool bBatchHasEndNode = false;
		for (const TObjectPtr<UTaskFlowNode>& NodePtr : Batch)
		{
			UTaskFlowNode* Node = NodePtr.Get();
			if (Node != nullptr && Node->IsA<UTaskFlowNode_End>())
			{
				// End nodes do not go through BeginExecution; emit the matching lifecycle log entries here.
				LogNodeLifecycle(this, Node, TEXT("Begin"), TEXT("EndNode"));
				Node->SetNodeState(ETaskFlowNodeState::Succeeded);
				LogNodeLifecycle(this, Node, TEXT("End"), TEXT("FlowEnded"));
				bBatchHasEndNode = true;
			}
		}

		if (bBatchHasEndNode)
		{
			// The End node has been reached: broadcast a completion event before shutting the flow down.
			// Stop() skips this path, so external listeners can distinguish completion from an aborted flow.
			EmitEventByName(TaskFlowConstants::EventName_Completed, this, Asset.Get() != nullptr ? Asset->GetName() : FString());

			// Terminate the whole flow: no other node in this batch is started; FinishFlow clears the active / pending sets
			//      and NotifyNodesFlowEnded makes every node in the graph unbind events, abort Actions and detach its Runner.
			FinishFlow();
			break;
		}

		for (const TObjectPtr<UTaskFlowNode>& NodePtr : Batch)
		{
			UTaskFlowNode* Node = NodePtr.Get();
			if (Node == nullptr)
			{
				continue;
			}

			// Multiple branches may schedule the same node into one batch; avoid starting it twice.
			if (ActiveNodes.ContainsByPredicate([Node](const TObjectPtr<UTaskFlowNode>& Ptr) { return Ptr.Get() == Node; }))
			{
				continue;
			}

			ActiveNodes.Add(Node);
			Node->SetNodeState(ETaskFlowNodeState::Executing);
			LogNodeLifecycle(this, Node, TEXT("Begin"), TEXT("Started"));
			Node->BeginExecution(this);
			// Synchronous nodes call FinishExecution -> NotifyNodeFinished here and remove themselves from ActiveNodes;
			//      their inner Advance() returns immediately because bAdvancing is set, so this while loop continues with the next batch.
		}

		if (StepDelaySeconds > 0.f)
		{
			// Step mode: exit the loop after handling one parallel batch; a timer schedules the next batch.
			break;
		}
	}

	bAdvancing = false;
}

void UTaskFlowRunner::FinishFlow()
{
	if (!bRunning)
	{
		return;
	}

	bRunning = false;
	PendingNodes.Empty();

	ClearAdvanceTimer();

	// When the flow ends (End / Stop), nodes still in the active set did not go through NotifyNodeFinished,
	//      so emit their matching End logs here before clearing the set.
	for (const TObjectPtr<UTaskFlowNode>& NodePtr : ActiveNodes)
	{
		LogNodeLifecycle(this, NodePtr.Get(), TEXT("End"), TEXT("Aborted"));
	}
	ActiveNodes.Empty();

	// Whole-flow completion: notify every node in the asset to abort pending async work (unbind global events, abort Actions)
	//      so leftover Objectives / Actions no longer react to events or advance after End / Stop.
	NotifyNodesFlowEnded();

	OnFinished.Broadcast(this);

	// Notify the outside world that the flow has finished (completed or aborted).
	EmitEventByName(TaskFlowConstants::EventName_Finished, this, Asset.Get() != nullptr ? Asset->GetName() : FString());

	// No longer keep the object alive via root; it still lives if external references (e.g. Blueprint variables) hold it.
	RemoveFromRoot();
}

void UTaskFlowRunner::NotifyNodesFlowEnded()
{
	if (Asset.Get() == nullptr)
	{
		return;
	}

	TArray<UTaskFlowNode*> AllNodes;
	Asset->GetAllNodes(AllNodes);
	for (UTaskFlowNode* Node : AllNodes)
	{
		if (Node == nullptr)
		{
			continue;
		}

		// Nodes interrupted by End / Stop while still Executing return to NotExecuted:
		//      they did not complete and should not stay yellow in the editor (FinishFlow emits the End logs).
		if (Node->GetNodeState() == ETaskFlowNodeState::Executing)
		{
			Node->SetNodeState(ETaskFlowNodeState::NotExecuted);
		}

		Node->OnFlowEnded();
	}
}

void UTaskFlowRunner::ScheduleAdvance()
{
	UWorld* World = GetTaskFlowWorld();
	if (World == nullptr)
	{
		// No World available; fall back to advancing immediately.
		Advance();
		return;
	}

	World->GetTimerManager().SetTimer(
		AdvanceTimerHandle,
		FTimerDelegate::CreateUObject(this, &UTaskFlowRunner::Advance),
		StepDelaySeconds,
		false);
}

void UTaskFlowRunner::ClearAdvanceTimer()
{
	if (AdvanceTimerHandle.IsValid())
	{
		UWorld* World = GetTaskFlowWorld();
		if (World != nullptr)
		{
			World->GetTimerManager().ClearTimer(AdvanceTimerHandle);
		}
	}
}

bool UTaskFlowRunner::SetBlackboardInt(FName Key, int32 Value)
{
	FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	Entry->IntValue = Value;
	return true;
}

bool UTaskFlowRunner::GetBlackboardInt(FName Key, int32& OutValue) const
{
	const FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	OutValue = Entry->IntValue;
	return true;
}

bool UTaskFlowRunner::SetBlackboardFloat(FName Key, float Value)
{
	FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	Entry->FloatValue = Value;
	return true;
}

bool UTaskFlowRunner::GetBlackboardFloat(FName Key, float& OutValue) const
{
	const FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	OutValue = Entry->FloatValue;
	return true;
}

bool UTaskFlowRunner::SetBlackboardBool(FName Key, bool Value)
{
	FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	Entry->BoolValue = Value;
	return true;
}

bool UTaskFlowRunner::GetBlackboardBool(FName Key, bool& OutValue) const
{
	const FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	OutValue = Entry->BoolValue;
	return true;
}

bool UTaskFlowRunner::SetBlackboardName(FName Key, FName Value)
{
	FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	Entry->NameValue = Value;
	return true;
}

bool UTaskFlowRunner::GetBlackboardName(FName Key, FName& OutValue) const
{
	const FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	OutValue = Entry->NameValue;
	return true;
}

bool UTaskFlowRunner::SetBlackboardString(FName Key, const FString& Value)
{
	FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	Entry->StringValue = Value;
	return true;
}

bool UTaskFlowRunner::GetBlackboardString(FName Key, FString& OutValue) const
{
	const FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	OutValue = Entry->StringValue;
	return true;
}

bool UTaskFlowRunner::SetBlackboardObject(FName Key, UObject* Value)
{
	FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	Entry->ObjectValue = Value;
	return true;
}

bool UTaskFlowRunner::GetBlackboardObject(FName Key, UObject*& OutValue) const
{
	const FTaskFlowBlackboardEntry* Entry = BlackboardValues.Find(Key);
	if (Entry == nullptr)
	{
		return false;
	}

	OutValue = Entry->ObjectValue;
	return true;
}

void UTaskFlowRunner::ValidateContextType(UTaskFlowAsset* InAsset, AActor* InContext)
{
#if WITH_EDITOR
	if (InAsset == nullptr || InContext == nullptr)
	{
		return;
	}

	const ETaskFlowTaskType TaskType = InAsset->TaskType;

	if (TaskType == ETaskFlowTaskType::Personal && Cast<AGameStateBase>(InContext) != nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("TaskFlowRunner", "TaskTypeMismatch_Personal", "This task flow is configured as Personal, but a GameState was passed in at runtime. The parameters do not match the configuration."));
	}
	else if (TaskType == ETaskFlowTaskType::World && Cast<APlayerState>(InContext) != nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("TaskFlowRunner", "TaskTypeMismatch_World", "This task flow is configured as World, but a PlayerState was passed in at runtime. The parameters do not match the configuration."));
	}
#endif
}
