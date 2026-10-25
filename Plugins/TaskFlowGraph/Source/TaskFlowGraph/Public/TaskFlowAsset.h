#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TaskFlowAsset.generated.h"

class UTaskFlowBlackboard;
class UTaskFlowGraph;
class UTaskFlowNode;
class UTaskFlowNode_Start;

/**
 * 任务流程资产：在内容浏览器里右键创建，双击打开后是一张 Graph，
 * 里面放置 Start / End 以及各种自定义 TaskFlowNode 并连线。
 */
UCLASS(BlueprintType, meta = (DisplayName = "任务流程"))
class TASKFLOWGRAPH_API UTaskFlowAsset : public UObject
{
	GENERATED_BODY()

public:
	UTaskFlowAsset();

	/** 图数据 */
	UPROPERTY()
	TObjectPtr<UTaskFlowGraph> Graph;

	/** 可选的黑板数据，节点/动作在运行时读写其中的值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	TObjectPtr<UTaskFlowBlackboard> Blackboard;

	UTaskFlowGraph* GetGraph() const { return Graph; }

	/** 入口节点 */
	UTaskFlowNode_Start* GetStartNode() const;

	/** 图中所有节点 */
	void GetAllNodes(TArray<UTaskFlowNode*>& OutNodes) const;

#if WITH_EDITOR
	/** 确保图存在，为空时创建并放置默认的 Start / End 节点 */
	UTaskFlowGraph* CreateOrLoadGraph();
#endif
};
