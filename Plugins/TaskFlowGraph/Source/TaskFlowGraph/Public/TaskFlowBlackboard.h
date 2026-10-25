#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TaskFlowBlackboard.generated.h"

/** 黑板值类型 */
UENUM(BlueprintType)
enum class ETaskFlowBlackboardValueType : uint8
{
	Int     UMETA(DisplayName = "整数"),
	Float   UMETA(DisplayName = "浮点"),
	Bool    UMETA(DisplayName = "布尔"),
	Name    UMETA(DisplayName = "名称"),
	String  UMETA(DisplayName = "字符串"),
	Object  UMETA(DisplayName = "对象")
};

/**
 * 黑板中的一个条目（属性）。
 * 既用于在 UTaskFlowBlackboard 里定义默认值，也用于保存运行时值。
 * 根据 ValueType 只使用对应的一个字段。
 */
USTRUCT(BlueprintType)
struct FTaskFlowBlackboardEntry
{
	GENERATED_BODY()

	/** 属性名（唯一标识） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName KeyName = NAME_None;

	/** 属性类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	ETaskFlowBlackboardValueType ValueType = ETaskFlowBlackboardValueType::Int;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Int", EditConditionHides))
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Float", EditConditionHides))
	float FloatValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Bool", EditConditionHides))
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Name", EditConditionHides))
	FName NameValue = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::String", EditConditionHides))
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (EditCondition = "ValueType == ETaskFlowBlackboardValueType::Object", EditConditionHides))
	TObjectPtr<UObject> ObjectValue = nullptr;
};

/**
 * 任务流程黑板数据资产。
 * 类似行为树黑板：开发者在这里添加若干带类型的属性条目，
 * 然后由 UTaskFlowAsset 引用，节点和动作在运行时读写这些值。
 */
UCLASS(BlueprintType)
class TASKFLOWGRAPH_API UTaskFlowBlackboard : public UObject
{
	GENERATED_BODY()

public:
	/** 黑板属性条目列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	TArray<FTaskFlowBlackboardEntry> Entries;

	/** 按 KeyName 查找条目，找不到返回 nullptr */
	const FTaskFlowBlackboardEntry* FindEntry(FName Key) const;
};
