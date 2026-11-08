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

#if WITH_EDITOR
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
			UE_LOG(LogTaskFlow, Warning, TEXT("[TaskFlow] Objective NodeName '%s' already exists; the change was reverted."), *OldNodeName.ToString());
		}

		// Node name changed; notify the editor to refresh the title display.
		UTaskFlowNode::OnNodeStateChanged().Broadcast(this);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UTaskFlowNode_Objective, ObjectiveDescription))
	{
		// Description changed; notify the editor to refresh the title display.
		UTaskFlowNode::OnNodeStateChanged().Broadcast(this);
	}
}
#endif

bool UTaskFlowNode_Objective::IsNodeNameDuplicate() const
{
	if (NodeName.IsNone())
	{
		// Unnamed nodes are not validated.
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
	// Prefer showing the objective description.
	if (!ObjectiveDescription.IsEmpty())
	{
		return ObjectiveDescription;
	}

	// Then the node name.
	if (!NodeName.IsNone())
	{
		return FText::FromName(NodeName);
	}

	// Finally fall back to the class name (friendly name after stripping the TaskFlowNode_ prefix, i.e. "Objective").
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

	// After paste the NodeName may collide with the original; clear it on collision to avoid uniqueness conflicts.
	if (IsNodeNameDuplicate())
	{
		NodeName = NAME_None;
	}
}

void UTaskFlowNode_Objective::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// Execute the Actions list from the beginning.
	CurrentActionIndex = 0;
	CurrentProgress = 0;
	bAdvancingActions = false;
	bActionsCompleted = false;
	bFlowEnded = false;

	// Global events are not listened to by default; bind only when needed (editor toggle / subclass override / runtime API).
	//      Either way, clear any binding left over from a previous execution first.
	UnbindFromTaskFlowEvent();

	if (ShouldListenToTaskFlowEvents())
	{
		if (UTaskFlowSystem* System = Runner->GetTaskFlowSystem())
		{
			BindToTaskFlowEvent(System);
		}
	}
	else
	{
		LogTaskFlowLifecycle(
			TEXT("<None>"), TEXT("EventSkipped"), GetName(),
			*GetClass()->GetName(), TEXT("Listening disabled"));
	}

	ExecuteNextAction();
}

void UTaskFlowNode_Objective::OnFlowEnded()
{
	// The whole flow has ended: this Objective no longer receives events, and late action callbacks cannot advance it.
	bFlowEnded = true;

	// Unbind using the cached System, not ActiveRunner (it may already be cleared when the flow ends).
	UnbindFromTaskFlowEvent();

	// Notify all Actions to stop (async Actions unbind their external listeners here and are marked finished).
	for (TObjectPtr<UTaskFlowAction>& ActionPtr : Actions)
	{
		if (UTaskFlowAction* Action = ActionPtr.Get())
		{
			Action->OnFlowEnded();
		}
	}

	// Reset execution state so nothing leaks into the next run of the same asset.
	CurrentActionIndex = 0;
	bAdvancingActions = false;
	bActionsCompleted = false;

	// Finally let the base class detach the Runner (FinishExecution no longer advances the flow afterwards).
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
		// A synchronous action's Finish triggered a recursive callback; let the outer loop continue.
		return;
	}

	bAdvancingActions = true;

	while (CurrentActionIndex < Actions.Num())
	{
		UTaskFlowAction* Action = Actions[CurrentActionIndex].Get();
		CurrentActionIndex++;

		if (Action == nullptr)
		{
			// Skip empty actions.
			continue;
		}

		Action->Start(this);

		if (!Action->IsFinished())
		{
			// Asynchronous action: wait for it to call Finish() later.
			bAdvancingActions = false;
			return;
		}
	}

	bAdvancingActions = false;

	// All actions done; combine with progress to decide real completion.
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
		// Still waiting for progress to reach the target.
		return;
	}

	// On completion, unbind from OnTaskFlowEvent using the cached System.
	UnbindFromTaskFlowEvent();

	FinishExecution();
}

bool UTaskFlowNode_Objective::ShouldListenToTaskFlowEvents() const
{
	return bListenToTaskFlowEvents;
}

void UTaskFlowNode_Objective::SetTaskFlowEventListening(bool bEnable)
{
	bListenToTaskFlowEvents = bEnable;

	// Only a node that is executing and whose flow has not ended may bind, to avoid leaving a global listener outside a flow.
	if (!bEnable || bFlowEnded || GetNodeState() != ETaskFlowNodeState::Executing)
	{
		UnbindFromTaskFlowEvent();
		return;
	}

	UnbindFromTaskFlowEvent();
	if (UTaskFlowSystem* System = GetTaskFlowSystem())
	{
		BindToTaskFlowEvent(System);
	}
}

void UTaskFlowNode_Objective::BindToTaskFlowEvent(UTaskFlowSystem* System)
{
	if (System == nullptr)
	{
		return;
	}

	System->OnTaskFlowEvent.AddDynamic(this, &UTaskFlowNode_Objective::HandleTaskFlowEvent);
	BoundSystem = System;

	LogTaskFlowLifecycle(
		TEXT("<None>"), TEXT("EventBound"), GetName(),
		*GetClass()->GetName(), TEXT("OnTaskFlowEvent"));
}

void UTaskFlowNode_Objective::UnbindFromTaskFlowEvent()
{
	// Use only the System cached at bind time: ActiveRunner may already be cleared when TryComplete / OnFlowEnded runs,
	//      so re-resolving via GetRunner()->GetTaskFlowSystem() would fail and leave the binding behind.
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
	// Ignore late progress updates after the flow ends, so leftover callbacks cannot change runtime state.
	if (bFlowEnded)
	{
		return;
	}

	CurrentProgress = Value;

	// Notify the editor to refresh the node (updates the progress display).
	UTaskFlowNode::OnNodeStateChanged().Broadcast(this);

	TryComplete();
}

void UTaskFlowNode_Objective::HandleTaskFlowEvent(const FTaskFlowEvent& Event)
{
	const bool bBlocked = bFlowEnded || GetNodeState() != ETaskFlowNodeState::Executing;
	const FString Detail = FString::Printf(TEXT("EventName=%s Blocked=%d"),
		*Event.EventName.ToString(), bBlocked ? 1 : 0);
	LogTaskFlowLifecycle(
		TEXT("<None>"), TEXT("EventReceived"), GetName(),
		*GetClass()->GetName(), *Detail);

	// Handle the event only while this node is executing and its flow has not ended
	//      (after the flow reaches End / is Stopped, a leftover event binding must not drive any action).
	if (bBlocked)
	{
		return;
	}

	// Pass the event to the subclass hook first (specialized Objectives may handle it directly), then forward it to unfinished Actions.
	OnTaskFlowEventReceived(Event);

	// The hook may already have completed this Objective / ended the flow, so check again before driving Actions.
	if (bFlowEnded || GetNodeState() != ETaskFlowNodeState::Executing)
	{
		return;
	}

	// The node listens itself and forwards events to unfinished Actions (the system no longer traverses internal nodes).
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
	// Default: no direct handling; specialized Objective subclasses override.
}

UTaskFlowSystem* UTaskFlowNode_Objective::GetTaskFlowSystem() const
{
	// Prefer the system resolved through the owning Objective's current Runner (most reliable).
	if (UTaskFlowRunner* Runner = this->GetRunner())
	{
		if (UTaskFlowSystem* System = Runner->GetTaskFlowSystem())
		{
			return System;
		}
	}

	// Fallback: resolve the world subsystem through this object's World context.
	return UTaskFlowSystem::Get(this);
}

#undef LOCTEXT_NAMESPACE
