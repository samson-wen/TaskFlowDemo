#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowAction.h"
#include "TaskFlowBlackboard.h"
#include "TaskFlowAction_SetBlackboardValue.generated.h"

/**
 * Action that sets a blackboard value.
 * Configure KeyName and ValueType; at execution the corresponding value is written into the blackboard.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Set Blackboard Value"))
class TASKFLOWGRAPH_API UTaskFlowAction_SetBlackboardValue : public UTaskFlowAction
{
	GENERATED_BODY()

public:
	UTaskFlowAction_SetBlackboardValue();

	virtual FText GetActionName() const override;
	virtual void Execute_Implementation(UTaskFlowNode_Objective* OwnerNode) override;

	/** The blackboard key to set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SetBlackboard")
	FName KeyName = NAME_None;

	/** Value type (should match the blackboard entry type). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SetBlackboard")
	ETaskFlowBlackboardValueType ValueType = ETaskFlowBlackboardValueType::Int;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SetBlackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Int", EditConditionHides))
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SetBlackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Float", EditConditionHides))
	float FloatValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SetBlackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Bool", EditConditionHides))
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SetBlackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Name", EditConditionHides))
	FName NameValue = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SetBlackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::String", EditConditionHides))
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SetBlackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Object", EditConditionHides))
	TObjectPtr<UObject> ObjectValue = nullptr;
};
