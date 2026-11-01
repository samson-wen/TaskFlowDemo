#include "Nodes/TaskFlowNode_End.h"

#include "TaskFlowRunner.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode_End"

void UTaskFlowNode_End::AllocateDefaultPins()
{
	// [EN] The End node only has an input pin.
	// [CN] End 节点只有输入引脚
	UEdGraphPin* InputPin = CreatePin(EGPD_Input, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Input);
	InputPin->PinFriendlyName = LOCTEXT("TaskFlowNode_End_InputPin", "In");
}

FText UTaskFlowNode_End::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("TaskFlowNode_End_Title", "End");
}

FText UTaskFlowNode_End::GetTooltipText() const
{
	return LOCTEXT("TaskFlowNode_End_Tooltip", "End of a branch");
}

void UTaskFlowNode_End::Execute_Implementation(UTaskFlowRunner* Runner)
{
	FinishExecution();
}

#undef LOCTEXT_NAMESPACE
