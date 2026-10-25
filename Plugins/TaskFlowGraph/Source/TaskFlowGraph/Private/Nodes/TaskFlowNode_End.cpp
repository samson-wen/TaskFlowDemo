#include "Nodes/TaskFlowNode_End.h"

#include "TaskFlowRunner.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode_End"

void UTaskFlowNode_End::AllocateDefaultPins()
{
	// End 节点只有输入引脚
	UEdGraphPin* InputPin = CreatePin(EGPD_Input, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Input);
	InputPin->PinFriendlyName = LOCTEXT("TaskFlowNode_End_InputPin", "到达");
}

FText UTaskFlowNode_End::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("TaskFlowNode_End_Title", "结束");
}

FText UTaskFlowNode_End::GetTooltipText() const
{
	return LOCTEXT("TaskFlowNode_End_Tooltip", "分支结束，流程走到这里即结束该分支");
}

void UTaskFlowNode_End::Execute_Implementation(UTaskFlowRunner* Runner)
{
	FinishExecution();
}

#undef LOCTEXT_NAMESPACE
