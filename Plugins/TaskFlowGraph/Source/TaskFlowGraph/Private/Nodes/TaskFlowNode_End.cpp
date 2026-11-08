#include "Nodes/TaskFlowNode_End.h"

#include "TaskFlowRunner.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode_End"

void UTaskFlowNode_End::AllocateDefaultPins()
{
	// The End node only has an input pin.
	UEdGraphPin* InputPin = CreatePin(EGPD_Input, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Input);
	InputPin->PinFriendlyName = LOCTEXT("TaskFlowNode_End_InputPin", "In");
}

FText UTaskFlowNode_End::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("TaskFlowNode_End_Title", "End");
}

FText UTaskFlowNode_End::GetTooltipText() const
{
	return LOCTEXT("TaskFlowNode_End_Tooltip", "End of the whole flow; stops all other executing nodes");
}

void UTaskFlowNode_End::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// The Runner detects End nodes before BeginExecution and terminates the flow immediately,
	// so this implementation is not reached during normal execution. The completion event is
	// broadcast from UTaskFlowRunner::Advance instead; keep this as a safe fallback.
	FinishExecution();
}

#undef LOCTEXT_NAMESPACE
