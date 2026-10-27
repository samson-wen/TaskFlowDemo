#include "TaskFlowRunner.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Misc/MessageDialog.h"
#include "Nodes/TaskFlowNode.h"
#include "Nodes/TaskFlowNode_End.h"
#include "Nodes/TaskFlowNode_Start.h"
#include "TaskFlowAsset.h"

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

	// 根据传入的上下文类型区分：PlayerState = 个人任务，GameState = 全局任务
	Context = InContext;
	PlayerStateOwner = Cast<APlayerState>(InContext);
	GameStateOwner = Cast<AGameStateBase>(InContext);

	// 校验上下文与资产任务类型是否一致（编辑器下弹窗提示）
	ValidateContextType(InAsset, InContext);

	Asset = InAsset;
	bRunning = true;

	// 从资产黑板初始化运行时黑板
	BlackboardValues.Empty();
	if (Asset->Blackboard.Get() != nullptr)
	{
		for (const FTaskFlowBlackboardEntry& Entry : Asset->Blackboard->Entries)
		{
			BlackboardValues.Add(Entry.KeyName, Entry);
		}
	}

	// 重置所有节点状态为未执行
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

	// 手动停止：把所有节点重置为未执行（灰色）
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

	// 标记节点执行成功
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

void UTaskFlowRunner::Advance()
{
	if (bAdvancing)
	{
		// 已经在推进中（同步节点里调用了 FinishExecution），交给外层循环继续
		return;
	}

	bAdvancing = true;

	while (bRunning)
	{
		if (CurrentNode.Get() != nullptr)
		{
			// 有异步节点还没完成，等它调用 FinishExecution
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
			// 到达 End 节点，标记成功，该分支结束
			Node->SetNodeState(ETaskFlowNodeState::Succeeded);
			continue;
		}

		CurrentNode = Node;
		Node->SetNodeState(ETaskFlowNodeState::Executing);
		Node->BeginExecution(this);

		if (StepDelaySeconds > 0.f)
		{
			// 步进模式：一个节点处理完就退出循环，由定时器安排下一个节点
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

	// 流程结束后不再持有，外部（蓝图变量等）仍持有引用时对象继续存活
	RemoveFromRoot();
}

void UTaskFlowRunner::ScheduleAdvance()
{
	UWorld* World = GetTaskFlowWorld();
	if (World == nullptr)
	{
		// 没有 World 时退化为立即推进
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
