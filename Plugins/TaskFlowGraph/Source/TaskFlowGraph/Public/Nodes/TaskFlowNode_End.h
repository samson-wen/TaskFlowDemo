#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_End.generated.h"

class UTaskFlowRunner;

/** [EN] Flow end node; there can be multiple, each marks the end of a branch. [CN] 流程结束节点，可以有多个，代表一条分支结束 */
UCLASS(meta = (DisplayName = "End", TaskFlowCategory = "Flow"))
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
