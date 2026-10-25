#include "Nodes/TaskFlowNode_Objective.h"

#include "Nodes/TaskFlowAction.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode_Objective"

UTaskFlowNode_Objective::UTaskFlowNode_Objective()
{
	ObjectiveDescription = LOCTEXT("TaskFlowNode_Objective_DefaultDescription", "完成此目标");
}

FText UTaskFlowNode_Objective::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("TaskFlowNode_Objective_Title", "目标");
}

FText UTaskFlowNode_Objective::GetTooltipText() const
{
	if (!ObjectiveDescription.IsEmpty())
	{
		return ObjectiveDescription;
	}

	return LOCTEXT("TaskFlowNode_Objective_Tooltip", "任务流程中的一个目标");
}

void UTaskFlowNode_Objective::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// 从头开始执行 Actions 列表
	CurrentActionIndex = 0;
	CurrentProgress = 0;
	bAdvancingActions = false;
	bActionsCompleted = false;

	ExecuteNextAction();
}

void UTaskFlowNode_Objective::ExecuteNextAction()
{
	if (bAdvancingActions)
	{
		// 同步动作里 Finish 触发了递归回调，交给外层循环继续推进
		return;
	}

	bAdvancingActions = true;

	while (CurrentActionIndex < Actions.Num())
	{
		UTaskFlowAction* Action = Actions[CurrentActionIndex].Get();
		CurrentActionIndex++;

		if (Action == nullptr)
		{
			// 空动作直接跳过
			continue;
		}

		Action->Start(this);

		if (!Action->IsFinished())
		{
			// 异步动作，等待它稍后调用 Finish()
			bAdvancingActions = false;
			return;
		}
	}

	bAdvancingActions = false;

	// 动作全部完成，再结合进度判断是否真正完成
	bActionsCompleted = true;
	TryComplete();
}

void UTaskFlowNode_Objective::OnActionFinished(UTaskFlowAction* Action)
{
	ExecuteNextAction();
}

void UTaskFlowNode_Objective::TryComplete()
{
	if (!bActionsCompleted)
	{
		return;
	}

	if (MaxProgress > 0 && CurrentProgress < MaxProgress)
	{
		// 还需要等待进度达标
		return;
	}

	FinishExecution();
}

void UTaskFlowNode_Objective::AddProgress(int32 Amount)
{
	SetProgress(CurrentProgress + Amount);
}

void UTaskFlowNode_Objective::SetProgress(int32 Value)
{
	CurrentProgress = Value;

	TryComplete();
}

#undef LOCTEXT_NAMESPACE
