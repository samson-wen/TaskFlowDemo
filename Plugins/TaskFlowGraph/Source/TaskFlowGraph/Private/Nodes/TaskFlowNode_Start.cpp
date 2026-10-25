#include "Nodes/TaskFlowNode_Start.h"

#include "TaskFlowRunner.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode_Start"

void UTaskFlowNode_Start::AllocateDefaultPins()
{
	// Start 节点只有输出引脚
	UEdGraphPin* OutputPin = CreatePin(EGPD_Output, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Output);
	OutputPin->PinFriendlyName = LOCTEXT("TaskFlowNode_Start_OutputPin", "开始");
}

FText UTaskFlowNode_Start::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("TaskFlowNode_Start_Title", "开始");
}

FText UTaskFlowNode_Start::GetTooltipText() const
{
	return LOCTEXT("TaskFlowNode_Start_Tooltip", "任务流程的入口，流程从这里开始执行");
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
