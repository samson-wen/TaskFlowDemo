#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TaskFlowAsset.generated.h"

class UTaskFlowBlackboard;
class UTaskFlowGraph;
class UTaskFlowNode;
class UTaskFlowNode_Start;

/** [EN] Task flow type. [CN] 任务流程类型 */
UENUM(BlueprintType)
enum class ETaskFlowTaskType : uint8
{
	/** [EN] World (global) task. [CN] 世界（全局）任务 */
	World    UMETA(DisplayName = "World"),
	/** [EN] Personal task. [CN] 个人任务 */
	Personal UMETA(DisplayName = "Personal"),
	/** [EN] Team task. [CN] 团队任务 */
	Team UMETA(DisplayName = "Team"),
	/** [EN] Faction task. [CN] 阵营任务 */
	Faction UMETA(DisplayName = "Faction")
};

/**
 * [EN] Task flow asset: created via right-click in the Content Browser; double-clicking opens a Graph
 * [EN] where you place Start / End and various custom TaskFlowNodes and wire them together.
 * [CN] 任务流程资产：在内容浏览器里右键创建，双击打开后是一张 Graph，
 * [CN] 里面放置 Start / End 以及各种自定义 TaskFlowNode 并连线。
 */
UCLASS(BlueprintType, meta = (DisplayName = "任务流程"))
class TASKFLOWGRAPH_API UTaskFlowAsset : public UObject
{
	GENERATED_BODY()

public:
	UTaskFlowAsset();

	/** [EN] Graph data. [CN] 图数据 */
	UPROPERTY()
	TObjectPtr<UTaskFlowGraph> Graph;

	/** [EN] Optional blackboard data; nodes/actions read and write its values at runtime. [CN] 可选的黑板数据，节点/动作在运行时读写其中的值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	TObjectPtr<UTaskFlowBlackboard> Blackboard;

	/** [EN] Task type: Personal / World. [CN] 任务类型：个人 / 世界 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	ETaskFlowTaskType TaskType = ETaskFlowTaskType::World;

	UTaskFlowGraph* GetGraph() const { return Graph; }

	/** [EN] Entry node. [CN] 入口节点 */
	UTaskFlowNode_Start* GetStartNode() const;

	/** [EN] All nodes in the graph. [CN] 图中所有节点 */
	void GetAllNodes(TArray<UTaskFlowNode*>& OutNodes) const;

#if WITH_EDITOR
	/** [EN] Ensure the graph exists; create it with default Start / End nodes when empty. [CN] 确保图存在，为空时创建并放置默认的 Start / End 节点 */
	UTaskFlowGraph* CreateOrLoadGraph();
#endif
};
