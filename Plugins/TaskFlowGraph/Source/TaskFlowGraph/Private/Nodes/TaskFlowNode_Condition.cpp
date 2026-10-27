#include "Nodes/TaskFlowNode_Condition.h"

#include "Nodes/TaskFlowNode_IF.h"

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

AActor* UTaskFlowNode_Condition::GetContext() const
{
	UTaskFlowNode_IF* Owner = OwnerIFNode.Get();
	return Owner != nullptr ? Owner->GetContext() : nullptr;
}

APlayerState* UTaskFlowNode_Condition::GetPlayerStateOwner() const
{
	UTaskFlowNode_IF* Owner = OwnerIFNode.Get();
	return Owner != nullptr ? Owner->GetPlayerStateOwner() : nullptr;
}

AGameStateBase* UTaskFlowNode_Condition::GetGameStateOwner() const
{
	UTaskFlowNode_IF* Owner = OwnerIFNode.Get();
	return Owner != nullptr ? Owner->GetGameStateOwner() : nullptr;
}
