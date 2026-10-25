#include "Nodes/TaskFlowNode_Condition_Blackboard.h"

#include "Nodes/TaskFlowNode_IF.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode_Condition_Blackboard"

FText UTaskFlowNode_Condition_Blackboard::GetConditionName() const
{
	return LOCTEXT("Condition_Blackboard_Name", "黑板比较条件");
}

bool UTaskFlowNode_Condition_Blackboard::Evaluate_Implementation(UTaskFlowNode_IF* OwnerNode)
{
	if (OwnerNode == nullptr)
	{
		return false;
	}

	for (const FTaskFlowBlackboardCheck& Check : Checks)
	{
		switch (Check.ValueType)
		{
		case ETaskFlowBlackboardValueType::Int:
		{
			int32 Value = 0;
			if (!OwnerNode->GetBlackboardInt(Check.KeyName, Value))
			{
				return false;
			}

			switch (Check.CompareOp)
			{
			case ETaskFlowBlackboardCompareOp::Greater:
				if (Value <= Check.IntValue) return false;
				break;
			case ETaskFlowBlackboardCompareOp::GreaterEqual:
				if (Value < Check.IntValue) return false;
				break;
			case ETaskFlowBlackboardCompareOp::Less:
				if (Value >= Check.IntValue) return false;
				break;
			case ETaskFlowBlackboardCompareOp::LessEqual:
				if (Value > Check.IntValue) return false;
				break;
			default:
				if (Value != Check.IntValue) return false;
				break;
			}
			break;
		}
		case ETaskFlowBlackboardValueType::Float:
		{
			float Value = 0.f;
			if (!OwnerNode->GetBlackboardFloat(Check.KeyName, Value))
			{
				return false;
			}

			switch (Check.CompareOp)
			{
			case ETaskFlowBlackboardCompareOp::Greater:
				if (Value <= Check.FloatValue) return false;
				break;
			case ETaskFlowBlackboardCompareOp::GreaterEqual:
				if (Value < Check.FloatValue) return false;
				break;
			case ETaskFlowBlackboardCompareOp::Less:
				if (Value >= Check.FloatValue) return false;
				break;
			case ETaskFlowBlackboardCompareOp::LessEqual:
				if (Value > Check.FloatValue) return false;
				break;
			default:
				if (Value != Check.FloatValue) return false;
				break;
			}
			break;
		}
		case ETaskFlowBlackboardValueType::Bool:
		{
			bool Value = false;
			if (!OwnerNode->GetBlackboardBool(Check.KeyName, Value) || Value != Check.BoolValue)
			{
				return false;
			}
			break;
		}
		case ETaskFlowBlackboardValueType::Name:
		{
			FName Value;
			if (!OwnerNode->GetBlackboardName(Check.KeyName, Value) || Value != Check.NameValue)
			{
				return false;
			}
			break;
		}
		case ETaskFlowBlackboardValueType::String:
		{
			FString Value;
			if (!OwnerNode->GetBlackboardString(Check.KeyName, Value) || Value != Check.StringValue)
			{
				return false;
			}
			break;
		}
		case ETaskFlowBlackboardValueType::Object:
		{
			UObject* Value = nullptr;
			if (!OwnerNode->GetBlackboardObject(Check.KeyName, Value) || Value != Check.ObjectValue.Get())
			{
				return false;
			}
			break;
		}
		default:
			return false;
		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
