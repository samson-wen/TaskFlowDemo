#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_End.generated.h"

class UTaskFlowRunner;

/**
 * [EN] Flow end node. When any End node is reached, the whole flow terminates immediately:
 * all other active / pending nodes in the same graph stop executing.
 * [CN] 流程结束节点。任一 End 节点被执行即终止整个流程：同一流程图中其它所有激活 / 待执行节点都会停止执行。
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
