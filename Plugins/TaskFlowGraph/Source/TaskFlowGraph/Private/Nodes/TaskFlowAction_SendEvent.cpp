#include "Nodes/TaskFlowAction_SendEvent.h"

#include "Nodes/TaskFlowNode.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowRunner.h"

// Action that emits an outbound event to the outside world when executed.
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
	// Route the event through the owning node's Runner so it reaches the external world.
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
