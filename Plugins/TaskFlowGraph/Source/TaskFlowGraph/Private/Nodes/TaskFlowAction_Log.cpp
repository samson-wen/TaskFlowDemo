#include "Nodes/TaskFlowAction_Log.h"

#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowAction_Log"

// Sample action that logs a message. Default message is set in the constructor.
UTaskFlowAction_Log::UTaskFlowAction_Log()
{
	Message = TEXT("Hello TaskFlow");
}

FText UTaskFlowAction_Log::GetActionName() const
{
	return LOCTEXT("TaskFlowAction_Log_Name", "Log");
}

void UTaskFlowAction_Log::Execute_Implementation(UTaskFlowNode_Objective* OwnerNode)
{
	UE_LOG(LogTaskFlow, Log, TEXT("[TaskFlow] %s"), *Message);

	Finish();
}

#undef LOCTEXT_NAMESPACE
