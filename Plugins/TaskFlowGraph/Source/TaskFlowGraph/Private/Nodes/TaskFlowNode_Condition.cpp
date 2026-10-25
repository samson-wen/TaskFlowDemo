#include "Nodes/TaskFlowNode_Condition.h"

FText UTaskFlowNode_Condition::GetConditionName() const
{
	FString Name = GetClass()->GetName();
	Name.RemoveFromStart(TEXT("TaskFlowNode_Condition_"));

	return FText::FromString(FName::NameToDisplayString(Name, false));
}

bool UTaskFlowNode_Condition::Evaluate_Implementation(UTaskFlowNode_IF* OwnerNode)
{
	// 默认实现：视为满足
	return true;
}

void UTaskFlowNode_Condition::Start(UTaskFlowNode_IF* InOwnerNode)
{
	OwnerIFNode = InOwnerNode;
	bEvaluated = true;
	bResult = Evaluate(InOwnerNode);
}
