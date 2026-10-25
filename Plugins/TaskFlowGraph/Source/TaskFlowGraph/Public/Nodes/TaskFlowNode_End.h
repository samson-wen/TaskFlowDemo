#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_End.generated.h"

class UTaskFlowRunner;

/** 流程结束节点，可以有多个，代表一条分支结束 */
UCLASS(meta = (DisplayName = "结束", TaskFlowCategory = "流程"))
class TASKFLOWGRAPH_API UTaskFlowNode_End : public UTaskFlowNode
{
	GENERATED_BODY()

public:
	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;
};
