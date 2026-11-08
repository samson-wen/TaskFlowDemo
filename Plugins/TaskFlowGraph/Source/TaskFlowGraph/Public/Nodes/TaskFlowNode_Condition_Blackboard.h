#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode_Condition.h"
#include "TaskFlowBlackboard.h"
#include "TaskFlowNode_Condition_Blackboard.generated.h"

/** Blackboard value comparison operation (Int/Float support Equal, Greater, Less; other types only Equal). */
UENUM(BlueprintType)
enum class ETaskFlowBlackboardCompareOp : uint8
{
	Equal        UMETA(DisplayName = "Equal"),
	Greater      UMETA(DisplayName = "Greater"),
	GreaterEqual UMETA(DisplayName = "Greater Equal"),
	Less         UMETA(DisplayName = "Less"),
	LessEqual    UMETA(DisplayName = "Less Equal")
};

/** Blackboard check item: Key + value type + comparison operation + comparison value. */
USTRUCT(BlueprintType)
struct FTaskFlowBlackboardCheck
{
	GENERATED_BODY()

	/** The blackboard key to compare. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	FName KeyName = NAME_None;

	/** Value type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	ETaskFlowBlackboardValueType ValueType = ETaskFlowBlackboardValueType::Int;

	/** Comparison operation, shown only for Int/Float. */
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
 * Blackboard comparison condition.
 * Configure one or more blackboard check items; at execution each is read from the blackboard and compared,
 * all must pass for the condition to hold; Int/Float support Equal, Greater, Less.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Blackboard Condition"))
class TASKFLOWGRAPH_API UTaskFlowNode_Condition_Blackboard : public UTaskFlowNode_Condition
{
	GENERATED_BODY()

public:
	virtual FText GetConditionName() const override;
	virtual bool Evaluate_Implementation(UTaskFlowNode_IF* OwnerNode) override;

	/** List of blackboard checks to evaluate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	TArray<FTaskFlowBlackboardCheck> Checks;
};
