#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_Start.generated.h"

class UTaskFlowRunner;

/** 流程入口节点，一张图里有且只有一个 */
UCLASS(meta = (DisplayName = "开始", TaskFlowCategory = "流程"))
class TASKFLOWGRAPH_API UTaskFlowNode_Start : public UTaskFlowNode
{
	GENERATED_BODY()

public:
	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface

	virtual bool IsUniqueNode() const override;

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;
};
