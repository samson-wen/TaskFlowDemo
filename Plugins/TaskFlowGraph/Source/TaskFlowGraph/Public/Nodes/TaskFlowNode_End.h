#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_End.generated.h"

class UTaskFlowRunner;

/**
 * Flow end node. When any End node is reached, the whole flow terminates immediately:
 * all other active / pending nodes in the same graph stop executing.
 */
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
