#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TaskFlowNode_Condition.generated.h"

class AActor;
class APlayerState;
class AGameStateBase;
class UTaskFlowNode_IF;

/**
 * [EN] Base class for conditions: an entry in the Conditions list of UTaskFlowNode_IF.
 * [CN] 条件基类：UTaskFlowNode_IF 的 Conditions 列表里的一项。
 * [EN] Developers inherit from this class (C++ or Blueprint) and implement Evaluate to define a custom condition;
 * [EN] returning true means the condition is satisfied.
 * [CN] 开发者继承本类（C++ 或蓝图）并实现 Evaluate 即可自定义条件，
 * [CN] Evaluate 返回 true 表示满足条件。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Condition"))
class TASKFLOWGRAPH_API UTaskFlowNode_Condition : public UObject
{
	GENERATED_BODY()

public:
	/** [EN] Condition display name; defaults to the class name (without the TaskFlowNode_Condition_ prefix). [CN] 条件显示名，默认取类名（去掉 TaskFlowNode_Condition_ 前缀） */
	virtual FText GetConditionName() const;

	/** [EN] Evaluate the condition; OwnerNode is the owning IF node. [CN] 评估条件，OwnerNode 为所属 IF 节点 */
	UFUNCTION(BlueprintNativeEvent, Category = "TaskFlow")
	bool Evaluate(UTaskFlowNode_IF* OwnerNode);

	/** [EN] Whether it has been evaluated. [CN] 是否已评估过 */
	bool IsEvaluated() const { return bEvaluated; }

	/** [EN] The most recent evaluation result. [CN] 最近一次评估结果 */
	bool GetResult() const { return bResult; }

	/** [EN] Internal: called by the IF node to trigger Evaluate. [CN] 内部：由 IF 节点调用，触发 Evaluate */
	void Start(UTaskFlowNode_IF* InOwnerNode);

	/** [EN] Get the context (PlayerState or GameState) evaluating this condition; may be null. [CN] 获取评估该条件的上下文（PlayerState 或 GameState），可能为空 */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AActor* GetContext() const;

	/** [EN] Get the PlayerState for a personal task (valid for personal tasks). [CN] 获取个人任务所属的 PlayerState（个人任务时有效） */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	APlayerState* GetPlayerStateOwner() const;

	/** [EN] Get the GameState for a global task (valid for global tasks). [CN] 获取全局任务所属的 GameState（全局任务时有效） */
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
