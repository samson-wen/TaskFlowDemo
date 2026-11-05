#include "Nodes/TaskFlowNode_Objective.h"

#include "EdGraph/EdGraph.h"
#include "Nodes/TaskFlowAction.h"
#include "TaskFlowRunner.h"
#include "TaskFlowSystem.h"
#include "TaskFlowTypes.h"
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
			UE_LOG(LogTaskFlow, Warning, TEXT("[TaskFlow] Objective NodeName '%s' 已存在，已回退。"), *OldNodeName.ToString());
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
	bFlowEnded = false;

	// [CN] 本节点执行期间绑定到系统的 OnTaskFlowEvent 委托，自行接收事件（系统不再遍历内部节点）。
	if (UTaskFlowSystem* System = Runner->GetTaskFlowSystem())
	{
		// [CN] 先移除旧绑定避免重复绑定（同一节点重复执行时），再绑定，并缓存所用 System。
		UnbindFromTaskFlowEvent();
		System->OnTaskFlowEvent.AddDynamic(this, &UTaskFlowNode_Objective::HandleTaskFlowEvent);
		BoundSystem = System;
	}

	ExecuteNextAction();
}

void UTaskFlowNode_Objective::OnFlowEnded()
{
	// [CN] 流程已整体结束：本目标不再接收事件，也不允许迟到的动作回调继续推进。
	bFlowEnded = true;

	// [CN] 用缓存的 System 解绑，不依赖 ActiveRunner（流程结束时它可能已被清空）。
	UnbindFromTaskFlowEvent();

	// [CN] 通知所有动作终止（异步动作会在此解绑自己的外部监听，并被标记为已完成）。
	for (TObjectPtr<UTaskFlowAction>& ActionPtr : Actions)
	{
		if (UTaskFlowAction* Action = ActionPtr.Get())
		{
			Action->OnFlowEnded();
		}
	}

	// [CN] 复位执行状态，避免同一资产再次启动流程时残留。
	CurrentActionIndex = 0;
	bAdvancingActions = false;
	bActionsCompleted = false;

	// [CN] 最后交给基类断开 Runner（此后 FinishExecution 不再推进流程）。
	Super::OnFlowEnded();
}

void UTaskFlowNode_Objective::ExecuteNextAction()
{
	if (bFlowEnded)
	{
		return;
	}

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
	if (bFlowEnded)
	{
		return;
	}

	ExecuteNextAction();
}

void UTaskFlowNode_Objective::TryComplete()
{
	if (bFlowEnded || !bActionsCompleted)
	{
		return;
	}

	if (MaxProgress > 0 && CurrentProgress < MaxProgress)
	{
		// [EN] Still waiting for progress to reach the target.
		// [CN] 还需要等待进度达标
		return;
	}

	// [CN] 完成时用缓存的 System 解除对 OnTaskFlowEvent 的绑定。
	UnbindFromTaskFlowEvent();

	FinishExecution();
}

void UTaskFlowNode_Objective::UnbindFromTaskFlowEvent()
{
	// [CN] 只使用绑定时缓存的 System：TryComplete / OnFlowEnded 触发时 ActiveRunner 可能已被清空，
	//      若改用 GetRunner()->GetTaskFlowSystem() 重新解析会拿不到 System，绑定就会残留。
	if (UTaskFlowSystem* System = BoundSystem.Get())
	{
		System->OnTaskFlowEvent.RemoveAll(this);
	}

	BoundSystem.Reset();
}

void UTaskFlowNode_Objective::AddProgress(int32 Amount)
{
	SetProgress(CurrentProgress + Amount);
}

void UTaskFlowNode_Objective::SetProgress(int32 Value)
{
	// [CN] 流程结束后忽略迟到的进度更新，避免残留回调改动运行时状态。
	if (bFlowEnded)
	{
		return;
	}

	CurrentProgress = Value;

	// [EN] Notify the editor to refresh the node (updates the progress display).
	// [CN] 通知编辑器刷新节点（更新进度显示）
	UTaskFlowNode::OnNodeStateChanged().Broadcast(this);

	TryComplete();
}

void UTaskFlowNode_Objective::HandleTaskFlowEvent(const FTaskFlowEvent& Event)
{
	// [CN] 仅在本节点正在执行、且所属流程尚未结束时处理
	//      （流程已到达 End / 被 Stop 后，残留的事件绑定不应再驱动任何动作）。
	if (bFlowEnded || GetNodeState() != ETaskFlowNodeState::Executing)
	{
		return;
	}

	// [CN] 先交给子类钩子（专用 Objective 可直接处理事件），再转发给尚未完成的动作。
	OnTaskFlowEventReceived(Event);

	// [CN] 钩子可能已经让本目标完成 / 流程结束，此处再校验一次，避免继续驱动动作。
	if (bFlowEnded || GetNodeState() != ETaskFlowNodeState::Executing)
	{
		return;
	}

	// [CN] 节点内自行监听并向尚未完成的动作转发事件（系统不再遍历内部节点，由各节点自行处理）。
	for (TObjectPtr<UTaskFlowAction>& ActionPtr : Actions)
	{
		UTaskFlowAction* Action = ActionPtr.Get();
		if (Action != nullptr && !Action->IsFinished())
		{
			Action->ReceiveTaskFlowEvent(Event);
		}
	}
}


void UTaskFlowNode_Objective::OnTaskFlowEventReceived(const FTaskFlowEvent& Event)
{
	// [EN] Default: no direct handling; specialized Objective subclasses override.
	// [CN] 默认不直接处理，专用目标子类重写。
}

UTaskFlowSystem* UTaskFlowNode_Objective::GetTaskFlowSystem() const
{
	// [CN] 优先经由所属目标节点当前执行的 Runner 取得系统（最可靠）。
	if (UTaskFlowRunner* Runner = this->GetRunner())
	{
		if (UTaskFlowSystem* System = Runner->GetTaskFlowSystem())
		{
			return System;
		}
	}

	// [CN] 回退：通过动作对象的 World 上下文直接取世界子系统。
	return UTaskFlowSystem::Get(this);
}

#undef LOCTEXT_NAMESPACE
