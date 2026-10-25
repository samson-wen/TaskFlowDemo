#include "Nodes/TaskFlowNode_IF.h"

#include "EdGraph/EdGraphPin.h"
#include "Nodes/TaskFlowNode_Condition.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode_IF"

void UTaskFlowNode_IF::AllocateDefaultPins()
{
	UEdGraphPin* InputPin = CreatePin(EGPD_Input, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Input);
	InputPin->PinFriendlyName = LOCTEXT("TaskFlowNode_IF_InputPin", "进入");

	UEdGraphPin* SuccessPin = CreatePin(EGPD_Output, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Success);
	SuccessPin->PinFriendlyName = LOCTEXT("TaskFlowNode_IF_SuccessPin", "成功");

	UEdGraphPin* FailedPin = CreatePin(EGPD_Output, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Failed);
	FailedPin->PinFriendlyName = LOCTEXT("TaskFlowNode_IF_FailedPin", "失败");
}

FText UTaskFlowNode_IF::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("TaskFlowNode_IF_Title", "条件分支");
}

FText UTaskFlowNode_IF::GetTooltipText() const
{
	return LOCTEXT("TaskFlowNode_IF_Tooltip", "满足所有条件走「成功」，否则走「失败」");
}

void UTaskFlowNode_IF::Execute_Implementation(UTaskFlowRunner* Runner)
{
	bLastResult = EvaluateConditions();

	FinishExecution();
}

bool UTaskFlowNode_IF::EvaluateConditions()
{
	for (UTaskFlowNode_Condition* Condition : Conditions)
	{
		if (Condition == nullptr)
		{
			continue;
		}

		Condition->Start(this);
		if (!Condition->GetResult())
		{
			return false;
		}
	}

	return true;
}

void UTaskFlowNode_IF::GetNextNodes(TArray<UTaskFlowNode*>& OutNextNodes) const
{
	OutNextNodes.Reset();

	const UEdGraphPin* OutputPin = bLastResult ? GetSuccessPin() : GetFailedPin();
	CollectNodesFromPin(OutputPin, OutNextNodes);
}

UEdGraphPin* UTaskFlowNode_IF::GetSuccessPin() const
{
	return FindPin(TaskFlowConstants::PinName_Success, EGPD_Output);
}

UEdGraphPin* UTaskFlowNode_IF::GetFailedPin() const
{
	return FindPin(TaskFlowConstants::PinName_Failed, EGPD_Output);
}

#undef LOCTEXT_NAMESPACE
