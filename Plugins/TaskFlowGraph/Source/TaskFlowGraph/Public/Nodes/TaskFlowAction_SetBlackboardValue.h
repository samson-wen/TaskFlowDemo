#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowAction.h"
#include "TaskFlowBlackboard.h"
#include "TaskFlowAction_SetBlackboardValue.generated.h"

/**
 * 设置黑板值的动作。
 * 配置 KeyName 与 ValueType，执行时把对应的值写入黑板。
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "设置黑板值"))
class TASKFLOWGRAPH_API UTaskFlowAction_SetBlackboardValue : public UTaskFlowAction
{
	GENERATED_BODY()

public:
	UTaskFlowAction_SetBlackboardValue();

	virtual FText GetActionName() const override;
	virtual void Execute_Implementation(UTaskFlowNode_Objective* OwnerNode) override;

	/** 要设置的黑板属性名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SetBlackboard")
	FName KeyName = NAME_None;

	/** 值类型（应与黑板条目类型一致） */
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
