#include "Nodes/TaskFlowNode_Objective.h"

#include "EdGraph/EdGraph.h"
#include "Nodes/TaskFlowAction.h"
#include "UObject/Field.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode_Objective"

UTaskFlowNode_Objective::UTaskFlowNode_Objective()
{
	ObjectiveDescription = LOCTEXT("TaskFlowNode_Objective_DefaultDescription", "Complete this objective");
}

void UTaskFlowNode_Objective::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange != nullptr && PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UTaskFlowNode_Objective, NodeName))
	{
		OldNodeName = NodeName;
	}
}

void UTaskFlowNode_Objective::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UTaskFlowNode_Objective, NodeName))
	{
		if (IsNodeNameDuplicate())
		{
			NodeName = OldNodeName;
			UE_LOG(LogTemp, Warning, TEXT("TaskFlowNode_Objective：NodeName '%s' 已存在，已回退。"), *OldNodeName.ToString());
		}

		// [EN] Node name changed; notify the editor to refresh the title display.
		// [CN] 节点名变化，通知编辑器刷新标题显示
		UTaskFlowNode::OnNodeStateChanged().Broadcast(this);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UTaskFlowNode_Objective, ObjectiveDescription))
	{
		// [EN] Description changed; notify the editor to refresh the title display.
		// [CN] 描述变化，通知编辑器刷新标题显示
		UTaskFlowNode::OnNodeStateChanged().Broadcast(this);
	}
}

bool UTaskFlowNode_Objective::IsNodeNameDuplicate() const
{
	if (NodeName.IsNone())
	{
		// [EN] Unnamed nodes are not validated.
		// [CN] 未命名不校验
		return false;
	}

	UEdGraph* Graph = GetGraph();
	if (Graph == nullptr)
	{
		return false;
	}

	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (Node == this)
		{
			continue;
		}

		const UTaskFlowNode_Objective* Other = Cast<UTaskFlowNode_Objective>(Node);
		if (Other != nullptr && Other->NodeName == NodeName)
		{
			return true;
		}
	}

	return false;
}

FText UTaskFlowNode_Objective::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	// [EN] Prefer showing the objective description.
	// [CN] 优先显示目标描述
	if (!ObjectiveDescription.IsEmpty())
	{
		return ObjectiveDescription;
	}

	// [EN] Then the node name.
	// [CN] 其次显示节点名称
	if (!NodeName.IsNone())
	{
		return FText::FromName(NodeName);
	}

	// [EN] Finally fall back to the class name (friendly name after stripping the TaskFlowNode_ prefix, i.e. "Objective").
	// [CN] 最后回退到类名（去掉 TaskFlowNode_ 前缀后的友好名，即 "Objective"）
	return Super::GetNodeTitle(TitleType);
}

FText UTaskFlowNode_Objective::GetTooltipText() const
{
	if (!ObjectiveDescription.IsEmpty())
	{
		return ObjectiveDescription;
	}

	return LOCTEXT("TaskFlowNode_Objective_Tooltip", "An objective in the task flow");
}

FName UTaskFlowNode_Objective::GetNodeName() const
{
	return NodeName;
}

void UTaskFlowNode_Objective::PostPasteNode()
{
	Super::PostPasteNode();

	// [EN] After paste the NodeName may collide with the original; clear it on collision to avoid uniqueness conflicts.
	// [CN] 粘贴后 NodeName 可能与原节点重复，重复则清空，避免唯一性冲突
	if (IsNodeNameDuplicate())
	{
		NodeName = NAME_None;
	}
}

void UTaskFlowNode_Objective::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// [EN] Execute the Actions list from the beginning.
	// [CN] 从头开始执行 Actions 列表
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
		// [EN] A synchronous action's Finish triggered a recursive callback; let the outer loop continue.
		// [CN] 同步动作里 Finish 触发了递归回调，交给外层循环继续推进
		return;
	}

	bAdvancingActions = true;

	while (CurrentActionIndex < Actions.Num())
	{
		UTaskFlowAction* Action = Actions[CurrentActionIndex].Get();
		CurrentActionIndex++;

		if (Action == nullptr)
		{
			// [EN] Skip empty actions.
			// [CN] 空动作直接跳过
			continue;
		}

		Action->Start(this);

		if (!Action->IsFinished())
		{
			// [EN] Asynchronous action: wait for it to call Finish() later.
			// [CN] 异步动作，等待它稍后调用 Finish()
			bAdvancingActions = false;
			return;
		}
	}

	bAdvancingActions = false;

	// [EN] All actions done; combine with progress to decide real completion.
	// [CN] 动作全部完成，再结合进度判断是否真正完成
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
		// [EN] Still waiting for progress to reach the target.
		// [CN] 还需要等待进度达标
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

	// [EN] Notify the editor to refresh the node (updates the progress display).
	// [CN] 通知编辑器刷新节点（更新进度显示）
	UTaskFlowNode::OnNodeStateChanged().Broadcast(this);

	TryComplete();
}

void UTaskFlowNode_Objective::ReceiveTaskFlowEvent_Implementation(const FTaskFlowEvent& Event)
{
	// [EN] No handling by default; override in subclasses / Blueprints.
	// [CN] 默认不处理，子类 / 蓝图重写
}

#undef LOCTEXT_NAMESPACE
