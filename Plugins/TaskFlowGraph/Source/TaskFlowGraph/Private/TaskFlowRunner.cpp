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
	 * [EN] The single formatter for every node lifecycle log line, so Begin and End always share the same layout.
	 * [CN] 节点生命周期日志的唯一格式化入口：Begin / End 共用同一套字段与顺序，保证格式统一。
	 * [EN] Format: [TaskFlow] Flow=<Asset> | Event=<Begin|End> | Node=<Name> | Class=<Class> | Detail=<...>
	 * [CN] 格式：[TaskFlow] Flow=<流程资产> | Event=<Begin|End> | Node=<节点名> | Class=<类名> | Detail=<附加信息>
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

	// [CN] 保留 ActiveNodes，交给 FinishFlow 统一为这些被中断的节点补 End 日志。
	PendingNodes.Empty();

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

	// [CN] 只接受当前活跃分支的完成回调；流程已结束或迟到的回调直接忽略。
	const int32 ActiveIndex = ActiveNodes.IndexOfByPredicate([Node](const TObjectPtr<UTaskFlowNode>& Ptr)
	{
		return Ptr.Get() == Node;
	});
	if (ActiveIndex == INDEX_NONE)
	{
		return;
	}

	// [CN] 仅移除完成的这一条分支，其它并行分支继续执行。
	ActiveNodes.RemoveAt(ActiveIndex, 1, EAllowShrinking::No);

	// [EN] Mark the node as succeeded.
	// [CN] 标记节点执行成功
	Node->SetNodeState(ETaskFlowNodeState::Succeeded);
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
	// [CN] 通过系统以 OnTaskFlowEvent 委托广播，所有订阅者（蓝图 / C++）都能收到。
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
		// [EN] Already advancing (a synchronous node called FinishExecution); let the outer loop continue.
		// [CN] 已经在推进中（同步节点里调用了 FinishExecution），交给外层循环继续
		return;
	}

	bAdvancing = true;

	while (bRunning)
	{
		// [CN] 没有待启动节点时：仍有活跃分支就等它们各自完成回调；活跃集合也空了才整条流程结束。
		if (PendingNodes.Num() == 0)
		{
			if (ActiveNodes.Num() == 0)
			{
				FinishFlow();
			}
			break;
		}

		// [CN] 把当前全部待启动节点取成一个批次，一齐开始执行。
		//      同步节点在 BeginExecution 内完成时会把后继加入新的 PendingNodes，由下一轮循环作为新批次处理。
		const TArray<TObjectPtr<UTaskFlowNode>> Batch = MoveTemp(PendingNodes);

		// [CN] 先整批判定 End：只要批次里出现 End，就代表整个流程结束。
		//      放在启动任何节点之前判定，行为不受 End 在批次中的先后顺序影响。
		bool bBatchHasEndNode = false;
		for (const TObjectPtr<UTaskFlowNode>& NodePtr : Batch)
		{
			UTaskFlowNode* Node = NodePtr.Get();
			if (Node != nullptr && Node->IsA<UTaskFlowNode_End>())
			{
				// [CN] End 节点不走 BeginExecution，在这里补上成对的生命周期日志。
				LogNodeLifecycle(this, Node, TEXT("Begin"), TEXT("EndNode"));
				Node->SetNodeState(ETaskFlowNodeState::Succeeded);
				LogNodeLifecycle(this, Node, TEXT("End"), TEXT("FlowEnded"));
				bBatchHasEndNode = true;
			}
		}

		if (bBatchHasEndNode)
		{
			// [CN] 终止整个流程：本批其它节点一律不启动；FinishFlow 会清空活跃 / 待执行集合，
			//      并通过 NotifyNodesFlowEnded 让图内所有节点解绑事件、中止 Actions、断开 Runner。
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

			// [CN] 多条分支可能把同一节点排进同一批次，避免重复启动。
			if (ActiveNodes.ContainsByPredicate([Node](const TObjectPtr<UTaskFlowNode>& Ptr) { return Ptr.Get() == Node; }))
			{
				continue;
			}

			ActiveNodes.Add(Node);
			Node->SetNodeState(ETaskFlowNodeState::Executing);
			LogNodeLifecycle(this, Node, TEXT("Begin"), TEXT("Started"));
			Node->BeginExecution(this);
			// [CN] 同步节点会在此调用 FinishExecution -> NotifyNodeFinished，把自己移出 ActiveNodes；
			//      其内部调用的 Advance() 因 bAdvancing 直接返回，由本 while 循环继续处理新批次。
		}

		if (StepDelaySeconds > 0.f)
		{
			// [EN] Step mode: exit the loop after handling one parallel batch; a timer schedules the next batch.
			// [CN] 步进模式：一个并行批次处理完就退出循环，由定时器安排下一批次
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

	// [CN] 流程结束（到达 End / Stop）时，仍留在活跃集合里的节点没有走 NotifyNodeFinished，
	//      在这里统一补上成对的 End 日志，再清空集合。
	for (const TObjectPtr<UTaskFlowNode>& NodePtr : ActiveNodes)
	{
		LogNodeLifecycle(this, NodePtr.Get(), TEXT("End"), TEXT("Aborted"));
	}
	ActiveNodes.Empty();

	// [CN] 流程整体结束：通知资产内所有节点终止未完成的异步工作（解绑全局事件、中止 Actions），
	//      保证到达 End / Stop 之后残留的 Objective/Actions 不会再被事件驱动或继续推进。
	NotifyNodesFlowEnded();

	OnFinished.Broadcast(this);

	// [EN] Notify the outside world that the flow has finished (completed or aborted).
	// [CN] 向外广播“流程已结束”事件（完成或中断都会触发）
	EmitEventByName(TaskFlowConstants::EventName_Finished, this, Asset.Get() != nullptr ? Asset->GetName() : FString());

	// [EN] No longer keep the object alive via root; it still lives if external references (e.g. Blueprint variables) hold it.
	// [CN] 流程结束后不再持有，外部（蓝图变量等）仍持有引用时对象继续存活
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

		// [CN] 被 End / Stop 打断、仍停留在「执行中」的节点回到「未执行」：
		//      它们并未完成，不应在编辑器里一直显示黄色（End 日志由 FinishFlow 统一输出）。
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
