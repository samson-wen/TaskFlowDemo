#include "Nodes/TaskFlowAction_Log.h"

#define LOCTEXT_NAMESPACE "TaskFlowAction_Log"

// [EN] Sample action that logs a message. Default message is set in the constructor.
// [CN] 示例动作：打印一条日志。构造函数设置了默认消息。
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
	UE_LOG(LogTemp, Log, TEXT("[TaskFlow] %s"), *Message);

	Finish();
}

#undef LOCTEXT_NAMESPACE
