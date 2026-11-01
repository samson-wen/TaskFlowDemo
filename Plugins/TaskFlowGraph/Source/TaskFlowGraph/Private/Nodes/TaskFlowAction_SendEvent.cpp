#include "Nodes/TaskFlowAction_SendEvent.h"

#include "Nodes/TaskFlowNode.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowRunner.h"

// [EN] Action that emits an outbound event to the outside world when executed.
// [CN] 执行时向外界发出一条出站事件的动作。
#define LOCTEXT_NAMESPACE "TaskFlowAction_SendEvent"

UTaskFlowAction_SendEvent::UTaskFlowAction_SendEvent()
{
}

FText UTaskFlowAction_SendEvent::GetActionName() const
{
	return LOCTEXT("TaskFlowAction_SendEvent_Name", "Send Event");
}

void UTaskFlowAction_SendEvent::Execute_Implementation(UTaskFlowNode_Objective* OwnerNode)
{
	// [EN] Route the event through the owning node's Runner so it reaches the external world.
	// [CN] 通过所属节点的 Runner 把事件转发到外界
	if (OwnerNode != nullptr)
	{
		if (UTaskFlowRunner* Runner = OwnerNode->GetRunner())
		{
			Runner->EmitEventByName(EventName, OwnerNode->GetContext(), Payload);
		}
	}

	Finish();
}

#undef LOCTEXT_NAMESPACE
