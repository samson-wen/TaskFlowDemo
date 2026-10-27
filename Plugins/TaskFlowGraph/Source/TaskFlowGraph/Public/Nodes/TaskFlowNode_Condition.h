#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TaskFlowNode_Condition.generated.h"

class AActor;
class APlayerState;
class AGameStateBase;
class UTaskFlowNode_IF;

/**
 * 条件基类：UTaskFlowNode_IF 的 Conditions 列表里的一项。
 * 开发者继承本类（C++ 或蓝图）并实现 Evaluate 即可自定义条件，
 * Evaluate 返回 true 表示满足条件。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "条件"))
class TASKFLOWGRAPH_API UTaskFlowNode_Condition : public UObject
{
	GENERATED_BODY()

public:
	/** 条件显示名，默认取类名（去掉 TaskFlowNode_Condition_ 前缀） */
	virtual FText GetConditionName() const;

	/** 评估条件，OwnerNode 为所属 IF 节点 */
	UFUNCTION(BlueprintNativeEvent, Category = "TaskFlow")
	bool Evaluate(UTaskFlowNode_IF* OwnerNode);

	/** 是否已评估过 */
	bool IsEvaluated() const { return bEvaluated; }

	/** 最近一次评估结果 */
	bool GetResult() const { return bResult; }

	/** 内部：由 IF 节点调用，触发 Evaluate */
	void Start(UTaskFlowNode_IF* InOwnerNode);

	/** 获取评估该条件的上下文（PlayerState 或 GameState），可能为空 */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AActor* GetContext() const;

	/** 获取个人任务所属的 PlayerState（个人任务时有效） */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	APlayerState* GetPlayerStateOwner() const;

	/** 获取全局任务所属的 GameState（全局任务时有效） */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AGameStateBase* GetGameStateOwner() const;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowNode_IF> OwnerIFNode;

	UPROPERTY(Transient)
	bool bEvaluated = false;

	UPROPERTY(Transient)
	bool bResult = false;
};
