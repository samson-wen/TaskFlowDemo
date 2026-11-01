#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode_Condition.h"
#include "TaskFlowBlackboard.h"
#include "TaskFlowNode_Condition_Blackboard.generated.h"

/** [EN] Blackboard value comparison operation (Int/Float support Equal, Greater, Less; other types only Equal). [CN] 黑板值比较操作（整数/浮点支持等于、大于、小于；其他类型只有等于） */
UENUM(BlueprintType)
enum class ETaskFlowBlackboardCompareOp : uint8
{
	Equal        UMETA(DisplayName = "Equal"),
	Greater      UMETA(DisplayName = "Greater"),
	GreaterEqual UMETA(DisplayName = "Greater Equal"),
	Less         UMETA(DisplayName = "Less"),
	LessEqual    UMETA(DisplayName = "Less Equal")
};

/** [EN] Blackboard check item: Key + value type + comparison operation + comparison value. [CN] 黑板比较项：Key + 值类型 + 比较操作 + 比较值 */
USTRUCT(BlueprintType)
struct FTaskFlowBlackboardCheck
{
	GENERATED_BODY()

	/** [EN] The blackboard key to compare. [CN] 要比较的黑板属性名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	FName KeyName = NAME_None;

	/** [EN] Value type. [CN] 值类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	ETaskFlowBlackboardValueType ValueType = ETaskFlowBlackboardValueType::Int;

	/** [EN] Comparison operation, shown only for Int/Float. [CN] 比较操作，仅整数/浮点显示 */
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
 * [EN] Blackboard comparison condition.
 * [CN] 黑板比较条件。
 * [EN] Configure one or more blackboard check items; at execution each is read from the blackboard and compared,
 * [EN] all must pass for the condition to hold; Int/Float support Equal, Greater, Less.
 * [CN] 配置一个或多个黑板比较项，执行时逐项读取黑板并比较，
 * [CN] 全部满足才算条件成立；整数/浮点支持等于、大于、小于。
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Blackboard Condition"))
class TASKFLOWGRAPH_API UTaskFlowNode_Condition_Blackboard : public UTaskFlowNode_Condition
{
	GENERATED_BODY()

public:
	virtual FText GetConditionName() const override;
	virtual bool Evaluate_Implementation(UTaskFlowNode_IF* OwnerNode) override;

	/** [EN] List of blackboard checks to evaluate. [CN] 要检查的黑板比较项列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	TArray<FTaskFlowBlackboardCheck> Checks;
};
