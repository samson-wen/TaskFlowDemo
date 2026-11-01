#include "Nodes/TaskFlowNode_Start.h"

#include "TaskFlowRunner.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode_Start"

void UTaskFlowNode_Start::AllocateDefaultPins()
{
	// [EN] The Start node only has an output pin.
	// [CN] Start 节点只有输出引脚
	UEdGraphPin* OutputPin = CreatePin(EGPD_Output, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Output);
	OutputPin->PinFriendlyName = LOCTEXT("TaskFlowNode_Start_OutputPin", "Start");
}

FText UTaskFlowNode_Start::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("TaskFlowNode_Start_Title", "Start");
}

FText UTaskFlowNode_Start::GetTooltipText() const
{
	return LOCTEXT("TaskFlowNode_Start_Tooltip", "Entry point of the task flow");
}

bool UTaskFlowNode_Start::IsUniqueNode() const
{
	return true;
}


void UTaskFlowNode_Start::Execute_Implementation(UTaskFlowRunner* Runner)
{
	FinishExecution();
}

#undef LOCTEXT_NAMESPACE
