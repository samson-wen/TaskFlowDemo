#include "Nodes/TaskFlowAction_SetBlackboardValue.h"

#include "Nodes/TaskFlowNode_Objective.h"

// [EN] Implementation of the "Set Blackboard Value" action. It writes the configured value into the blackboard at execution, then finishes.
// [CN] “设置黑板值”动作的实现。执行时把配置的值写入黑板，然后完成。
#define LOCTEXT_NAMESPACE "TaskFlowAction_SetBlackboardValue"

UTaskFlowAction_SetBlackboardValue::UTaskFlowAction_SetBlackboardValue()
{
}

FText UTaskFlowAction_SetBlackboardValue::GetActionName() const
{
	return LOCTEXT("TaskFlowAction_SetBlackboardValue_Name", "Set Blackboard Value");
}

void UTaskFlowAction_SetBlackboardValue::Execute_Implementation(UTaskFlowNode_Objective* OwnerNode)
{
	if (OwnerNode != nullptr)
	{
		switch (ValueType)
		{
		case ETaskFlowBlackboardValueType::Int:
			OwnerNode->SetBlackboardInt(KeyName, IntValue);
			break;
		case ETaskFlowBlackboardValueType::Float:
			OwnerNode->SetBlackboardFloat(KeyName, FloatValue);
			break;
		case ETaskFlowBlackboardValueType::Bool:
			OwnerNode->SetBlackboardBool(KeyName, BoolValue);
			break;
		case ETaskFlowBlackboardValueType::Name:
			OwnerNode->SetBlackboardName(KeyName, NameValue);
			break;
		case ETaskFlowBlackboardValueType::String:
			OwnerNode->SetBlackboardString(KeyName, StringValue);
			break;
		case ETaskFlowBlackboardValueType::Object:
			OwnerNode->SetBlackboardObject(KeyName, ObjectValue);
			break;
		default:
			break;
		}
	}

	Finish();
}

#undef LOCTEXT_NAMESPACE
