#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_IF.generated.h"

class UEdGraphPin;
class UTaskFlowNode_Condition;
class UTaskFlowRunner;

/**
 * [EN] Conditional branch node.
 * [CN] 条件分支节点。
 * [EN] On execution it evaluates all conditions in the Conditions list in order: if all are satisfied it follows the "Success" output, otherwise the "Failed" output.
 * [CN] 执行时依次评估 Conditions 列表里的所有条件：全部满足走「成功」输出，否则走「失败」输出。
 */
UCLASS(meta = (DisplayName = "Condition Branch", TaskFlowCategory = "Flow"))
class TASKFLOWGRAPH_API UTaskFlowNode_IF : public UTaskFlowNode
{
	GENERATED_BODY()

public:
	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;

	virtual void GetNextNodes(TArray<UTaskFlowNode*>& OutNextNodes) const override;

	/** [EN] Condition list; all satisfied -> Success, otherwise -> Failed. [CN] 条件列表，全部满足走 Success，否则走 Failed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IF", Instanced)
	TArray<TObjectPtr<UTaskFlowNode_Condition>> Conditions;

	/** [EN] Success output pin. [CN] Success 输出引脚 */
	UEdGraphPin* GetSuccessPin() const;

	/** [EN] Failed output pin. [CN] Failed 输出引脚 */
	UEdGraphPin* GetFailedPin() const;

private:
	/** [EN] Evaluate all conditions in order; returns true only if all are satisfied. [CN] 依次执行所有条件，全部满足返回 true */
	bool EvaluateConditions();

	/** [EN] The most recent execution result (decides which output to follow). [CN] 最近一次执行结果（决定走哪个输出） */
	bool bLastResult = false;
};
