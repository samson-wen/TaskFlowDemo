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

UTaskFlowRunner* UTaskFlowRunner::StartTaskFlow(const UObject* WorldContextObject, UTaskFlowAsset* InAsset, AActor* InContext)
{
	if (InAsset == nullptr)
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

	// [EN] Distinguish by the context type passed in: PlayerState = personal task, GameState = global task.
	// [CN] 根据传入的上下文类型区分：PlayerState = 个人任务，GameState = 全局任务
	Context = InContext;
	PlayerStateOwner = Cast<APlayerState>(InContext);
	GameStateOwner = Cast<AGameStateBase>(InContext);

	// [EN] Validate that the context matches the asset's task type (editor popup).
	// [CN] 校验上下文与资产任务类型是否一致（编辑器下弹窗提示）
	ValidateContextType(InAsset, InContext);

	Asset = InAsset;
	bRunning = true;

	// [EN] Initialize the runtime blackboard from the asset blackboard.
	// [CN] 从资产黑板初始化运行时黑板
	BlackboardValues.Empty();
	if (Asset->Blackboard.Get() != nullptr)
	{
		for (const FTaskFlowBlackboardEntry& Entry : Asset->Blackboard->Entries)
		{
			BlackboardValues.Add(Entry.KeyName, Entry);
		}
	}

	// [EN] Reset all node states to NotExecuted.
	// [CN] 重置所有节点状态为未执行
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

	// [EN] Notify the outside world that the flow has started (before Advance so it always precedes the Finished event, even for synchronous flows).
	// [CN] 向外广播“流程已开始”事件（在 Advance 之前，确保即便同步流程也先于“结束”事件发出）
	EmitEventByName(TaskFlowConstants::EventName_Started, this, Asset.Get() != nullptr ? Asset->GetName() : FString());

	Advance();
}

void UTaskFlowRunner::Stop()
{
	if (!bRunning)
	{
		return;
	}

	PendingNodes.Empty();
	CurrentNode = nullptr;

	ClearAdvanceTimer();

	// [EN] Manual stop: reset all nodes to NotExecuted (gray).
	// [CN] 手动停止：把所有节点重置为未执行（灰色）
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
	return CurrentNode;
}

UWorld* UTaskFlowRunner::GetTaskFlowWorld() const
{
	UWorld* World = WorldPtr.Get();
	return World != nullptr ? World : GetWorld();
}

void UTaskFlowRunner::NotifyNodeFinished(UTaskFlowNode* Node)
{
	if (!bRunning || Node == nullptr || CurrentNode.Get() != Node)
	{
		return;
	}

	CurrentNode = nullptr;

	// [EN] Mark the node as succeeded.
	// [CN] 标记节点执行成功
	Node->SetNodeState(ETaskFlowNodeState::Succeeded);

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

void UTaskFlowRunner::DispatchEvent(const FTaskFlowEvent& Event)
{
	if (Asset.Get() == nullptr)
	{
		return;
	}

	TArray<UTaskFlowNode*> AllNodes;
	Asset->GetAllNodes(AllNodes);

	for (UTaskFlowNode* Node : AllNodes)
	{
		UTaskFlowNode_Objective* Objective = Cast<UTaskFlowNode_Objective>(Node);
		if (Objective == nullptr || Objective->GetNodeState() != ETaskFlowNodeState::Executing)
		{
			continue;
		}

		// [EN] Only process Objectives that have event listening enabled, together with their unfinished actions.
		// [CN] 只有目标节点开启了事件监听，才处理它本身及其未完成的动作
		if (!Objective->bListenEvent)
		{
			continue;
		}

		ITaskFlowEventReceiver::Execute_ReceiveTaskFlowEvent(Objective, Event);

		// [EN] Also dispatch to its actions that have not finished yet.
		// [CN] 同时执行其尚未完成的动作
		for (TObjectPtr<UTaskFlowAction>& ActionPtr : Objective->Actions)
		{
			UTaskFlowAction* Action = ActionPtr.Get();
			if (Action != nullptr && !Action->IsFinished())
			{
				ITaskFlowEventReceiver::Execute_ReceiveTaskFlowEvent(Action, Event);
			}
		}
	}
}

void UTaskFlowRunner::EmitEvent(const FTaskFlowEvent& Event)
{
	// [EN] Route the outbound event through the system so external listeners (global delegate + registered receivers) get it.
	// [CN] 把出站事件交给 System 转发，使外部监听者（全局委托 + 已注册接收者）都能收到
	UTaskFlowSystem* System = GetTaskFlowSystem();
	if (System != nullptr)
	{
		System->BroadcastEventToExternal(Event);
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
		// [EN] Already advancing (a synchronous node called FinishExecution); let the outer loop continue.
		// [CN] 已经在推进中（同步节点里调用了 FinishExecution），交给外层循环继续
		return;
	}

	bAdvancing = true;

	while (bRunning)
	{
		if (CurrentNode.Get() != nullptr)
		{
			// [EN] An asynchronous node has not finished; wait for it to call FinishExecution.
			// [CN] 有异步节点还没完成，等它调用 FinishExecution
			break;
		}

		if (PendingNodes.Num() == 0)
		{
			FinishFlow();
			break;
		}

		UTaskFlowNode* Node = PendingNodes[0].Get();
		PendingNodes.RemoveAt(0, 1, EAllowShrinking::No);

		if (Node == nullptr)
		{
			continue;
		}

		if (Node->IsA<UTaskFlowNode_End>())
		{
			// [EN] Reached an End node: mark it succeeded and end this branch.
			// [CN] 到达 End 节点，标记成功，该分支结束
			Node->SetNodeState(ETaskFlowNodeState::Succeeded);
			continue;
		}

		CurrentNode = Node;
		Node->SetNodeState(ETaskFlowNodeState::Executing);
		Node->BeginExecution(this);

		if (StepDelaySeconds > 0.f)
		{
			// [EN] Step mode: exit the loop after handling one node; a timer schedules the next node.
			// [CN] 步进模式：一个节点处理完就退出循环，由定时器安排下一个节点
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
	CurrentNode = nullptr;
	PendingNodes.Empty();

	ClearAdvanceTimer();

	OnFinished.Broadcast(this);

	// [EN] Notify the outside world that the flow has finished (completed or aborted).
	// [CN] 向外广播“流程已结束”事件（完成或中断都会触发）
	EmitEventByName(TaskFlowConstants::EventName_Finished, this, Asset.Get() != nullptr ? Asset->GetName() : FString());

	// [EN] No longer keep the object alive via root; it still lives if external references (e.g. Blueprint variables) hold it.
	// [CN] 流程结束后不再持有，外部（蓝图变量等）仍持有引用时对象继续存活
	RemoveFromRoot();
}

void UTaskFlowRunner::ScheduleAdvance()
{
	UWorld* World = GetTaskFlowWorld();
	if (World == nullptr)
	{
		// [EN] No World available; fall back to advancing immediately.
		// [CN] 没有 World 时退化为立即推进
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
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("TaskFlowRunner", "TaskTypeMismatch_Personal", "该任务流程设置为「个人」任务，但运行时传入的是 GameState，参数与设置不一致。"));
	}
	else if (TaskType == ETaskFlowTaskType::World && Cast<APlayerState>(InContext) != nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("TaskFlowRunner", "TaskTypeMismatch_World", "该任务流程设置为「世界」任务，但运行时传入的是 PlayerState，参数与设置不一致。"));
	}
#endif
}
