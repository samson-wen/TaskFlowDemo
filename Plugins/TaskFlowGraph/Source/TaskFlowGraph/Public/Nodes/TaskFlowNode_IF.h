#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_IF.generated.h"

class UEdGraphPin;
class UTaskFlowNode_Condition;
class UTaskFlowRunner;

/**
 * Conditional branch node.
 * On execution it evaluates all conditions in the Conditions list in order: if all are satisfied it follows the "Success" output, otherwise the "Failed" output.
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

	/** Condition list; all satisfied -> Success, otherwise -> Failed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IF", Instanced)
	TArray<TObjectPtr<UTaskFlowNode_Condition>> Conditions;

	/** Success output pin. */
	UEdGraphPin* GetSuccessPin() const;

	/** Failed output pin. */
	UEdGraphPin* GetFailedPin() const;

private:
	/** Evaluate all conditions in order; returns true only if all are satisfied. */
	bool EvaluateConditions();

	/** The most recent execution result (decides which output to follow). */
	bool bLastResult = false;
};
