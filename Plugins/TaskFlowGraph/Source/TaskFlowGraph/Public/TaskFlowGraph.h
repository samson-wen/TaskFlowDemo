#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "TaskFlowGraph.generated.h"

class UTaskFlowAsset;
class UTaskFlowNode;
class UTaskFlowNode_Start;

/**
 * 任务流程图，保存在 UTaskFlowAsset 里，编辑器中通过 Graph 编辑器打开。
 */
UCLASS()
class TASKFLOWGRAPH_API UTaskFlowGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UTaskFlowGraph();

	/** 拥有这张图的资产 */
	UTaskFlowAsset* GetTaskFlowAsset() const;

	/** 入口节点 */
	UTaskFlowNode_Start* GetStartNode() const;

	/** 图中所有任务流节点 */
	void GetAllNodes(TArray<UTaskFlowNode*>& OutNodes) const;

#if WITH_EDITOR
	/** 创建默认节点（Start / End） */
	void CreateDefaultNodes();
#endif
};
