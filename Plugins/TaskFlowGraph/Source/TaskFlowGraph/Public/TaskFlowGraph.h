#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "TaskFlowGraph.generated.h"

class UTaskFlowAsset;
class UTaskFlowNode;
class UTaskFlowNode_Start;

/**
 * Task flow graph, stored inside a UTaskFlowAsset and opened in the Graph editor.
 */
UCLASS()
class TASKFLOWGRAPH_API UTaskFlowGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UTaskFlowGraph();

	/** The asset that owns this graph. */
	UTaskFlowAsset* GetTaskFlowAsset() const;

	/** Entry node. */
	UTaskFlowNode_Start* GetStartNode() const;

	/** All task flow nodes in the graph. */
	void GetAllNodes(TArray<UTaskFlowNode*>& OutNodes) const;

#if WITH_EDITOR
	/** Create default nodes (Start / End). */
	void CreateDefaultNodes();
#endif
};
