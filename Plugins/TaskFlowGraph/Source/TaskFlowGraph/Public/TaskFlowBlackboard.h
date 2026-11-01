#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TaskFlowBlackboard.generated.h"

/** [EN] Blackboard value type. [CN] 黑板值类型 */
UENUM(BlueprintType)
enum class ETaskFlowBlackboardValueType : uint8
{
	Int     UMETA(DisplayName = "Int"),
	Float   UMETA(DisplayName = "Float"),
	Bool    UMETA(DisplayName = "Bool"),
	Name    UMETA(DisplayName = "Name"),
	String  UMETA(DisplayName = "String"),
	Object  UMETA(DisplayName = "Object")
};

/**
 * [EN] A single entry (property) in the blackboard.
 * [CN] 黑板中的一个条目（属性）。
 * [EN] Used both to define default values in UTaskFlowBlackboard and to hold runtime values.
 * [CN] 既用于在 UTaskFlowBlackboard 里定义默认值，也用于保存运行时值。
 * [EN] Only the field matching ValueType is used.
 * [CN] 根据 ValueType 只使用对应的一个字段。
 */
USTRUCT(BlueprintType)
struct FTaskFlowBlackboardEntry
{
	GENERATED_BODY()

	/** [EN] Property name (unique identifier). [CN] 属性名（唯一标识） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName KeyName = NAME_None;

	/** [EN] Property type. [CN] 属性类型 */
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
 * [EN] Task flow blackboard data asset.
 * [CN] 任务流程黑板数据资产。
 * [EN] Similar to a behavior-tree blackboard: developers add several typed property entries here,
 * [EN] then reference it from a UTaskFlowAsset; nodes and actions read/write these values at runtime.
 * [CN] 类似行为树黑板：开发者在这里添加若干带类型的属性条目，
 * [CN] 然后由 UTaskFlowAsset 引用，节点和动作在运行时读写这些值。
 */
UCLASS(BlueprintType)
class TASKFLOWGRAPH_API UTaskFlowBlackboard : public UObject
{
	GENERATED_BODY()

public:
	/** [EN] List of blackboard property entries. [CN] 黑板属性条目列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	TArray<FTaskFlowBlackboardEntry> Entries;

	/** [EN] Find an entry by KeyName; returns nullptr if not found. [CN] 按 KeyName 查找条目，找不到返回 nullptr */
	const FTaskFlowBlackboardEntry* FindEntry(FName Key) const;
};
