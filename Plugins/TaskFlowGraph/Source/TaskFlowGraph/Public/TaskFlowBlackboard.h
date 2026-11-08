#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TaskFlowBlackboard.generated.h"

/** Blackboard value type. */
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
 * A single entry (property) in the blackboard.
 * Used both to define default values in UTaskFlowBlackboard and to hold runtime values.
 * Only the field matching ValueType is used.
 */
USTRUCT(BlueprintType)
struct FTaskFlowBlackboardEntry
{
	GENERATED_BODY()

	/** Property name (unique identifier). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName KeyName = NAME_None;

	/** Property type. */
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
 * Task flow blackboard data asset.
 * Similar to a behavior-tree blackboard: developers add several typed property entries here,
 * then reference it from a UTaskFlowAsset; nodes and actions read/write these values at runtime.
 */
UCLASS(BlueprintType)
class TASKFLOWGRAPH_API UTaskFlowBlackboard : public UObject
{
	GENERATED_BODY()

public:
	/** List of blackboard property entries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	TArray<FTaskFlowBlackboardEntry> Entries;

	/** Find an entry by KeyName; returns nullptr if not found. */
	const FTaskFlowBlackboardEntry* FindEntry(FName Key) const;
};
