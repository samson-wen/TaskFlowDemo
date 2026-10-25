#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode_Condition.h"
#include "TaskFlowBlackboard.h"
#include "TaskFlowNode_Condition_Blackboard.generated.h"

/** 黑板值比较操作（整数/浮点支持等于、大于、小于；其他类型只有等于） */
UENUM(BlueprintType)
enum class ETaskFlowBlackboardCompareOp : uint8
{
	Equal        UMETA(DisplayName = "等于"),
	Greater      UMETA(DisplayName = "大于"),
	GreaterEqual UMETA(DisplayName = "大于等于"),
	Less         UMETA(DisplayName = "小于"),
	LessEqual    UMETA(DisplayName = "小于等于")
};

/** 黑板比较项：Key + 值类型 + 比较操作 + 比较值 */
USTRUCT(BlueprintType)
struct FTaskFlowBlackboardCheck
{
	GENERATED_BODY()

	/** 要比较的黑板属性名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	FName KeyName = NAME_None;

	/** 值类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	ETaskFlowBlackboardValueType ValueType = ETaskFlowBlackboardValueType::Int;

	/** 比较操作，仅整数/浮点显示 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Int || ValueType == ETaskFlowBlackboardValueType::Float", EditConditionHides))
	ETaskFlowBlackboardCompareOp CompareOp = ETaskFlowBlackboardCompareOp::Equal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Int", EditConditionHides))
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Float", EditConditionHides))
	float FloatValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Bool", EditConditionHides))
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Name", EditConditionHides))
	FName NameValue = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::String", EditConditionHides))
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Object", EditConditionHides))
	TObjectPtr<UObject> ObjectValue = nullptr;
};

/**
 * 黑板比较条件。
 * 配置一个或多个黑板比较项，执行时逐项读取黑板并比较，
 * 全部满足才算条件成立；整数/浮点支持等于、大于、小于。
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "黑板比较条件"))
class TASKFLOWGRAPH_API UTaskFlowNode_Condition_Blackboard : public UTaskFlowNode_Condition
{
	GENERATED_BODY()

public:
	virtual FText GetConditionName() const override;
	virtual bool Evaluate_Implementation(UTaskFlowNode_IF* OwnerNode) override;

	/** 要检查的黑板比较项列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	TArray<FTaskFlowBlackboardCheck> Checks;
};
