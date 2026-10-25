#include "Nodes/TaskFlowAction_Log.h"

#define LOCTEXT_NAMESPACE "TaskFlowAction_Log"

UTaskFlowAction_Log::UTaskFlowAction_Log()
{
	Message = TEXT("Hello TaskFlow");
}

FText UTaskFlowAction_Log::GetActionName() const
{
	return LOCTEXT("TaskFlowAction_Log_Name", "打印日志");
}

void UTaskFlowAction_Log::Execute_Implementation(UTaskFlowNode_Objective* OwnerNode)
{
	UE_LOG(LogTemp, Log, TEXT("[TaskFlow] %s"), *Message);

	Finish();
}

#undef LOCTEXT_NAMESPACE
