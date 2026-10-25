#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_IF.generated.h"

class UEdGraphPin;
class UTaskFlowNode_Condition;
class UTaskFlowRunner;

/**
 * 条件分支节点。
 * 执行时依次评估 Conditions 列表里的所有条件：全部满足走「成功」输出，否则走「失败」输出。
 */
UCLASS(meta = (DisplayName = "条件分支", TaskFlowCategory = "流程"))
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

	/** 条件列表，全部满足走 Success，否则走 Failed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IF", Instanced)
	TArray<TObjectPtr<UTaskFlowNode_Condition>> Conditions;

	/** Success 输出引脚 */
	UEdGraphPin* GetSuccessPin() const;

	/** Failed 输出引脚 */
	UEdGraphPin* GetFailedPin() const;

private:
	/** 依次执行所有条件，全部满足返回 true */
	bool EvaluateConditions();

	/** 最近一次执行结果（决定走哪个输出） */
	bool bLastResult = false;
};
