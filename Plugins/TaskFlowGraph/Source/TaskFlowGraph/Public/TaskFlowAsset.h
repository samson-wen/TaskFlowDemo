#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TaskFlowAsset.generated.h"

class UTaskFlowBlackboard;
class UTaskFlowGraph;
class UTaskFlowNode;
class UTaskFlowNode_Start;

/** Task flow type. */
UENUM(BlueprintType)
enum class ETaskFlowTaskType : uint8
{
	/** World (global) task. */
	World    UMETA(DisplayName = "World"),
	/** Personal task. */
	Personal UMETA(DisplayName = "Personal"),
	/** Team task. */
	Team UMETA(DisplayName = "Team"),
	/** Faction task. */
	Faction UMETA(DisplayName = "Faction")
};

/**
 * Task flow asset: created via right-click in the Content Browser; double-clicking opens a Graph
 * where you place Start / End and various custom TaskFlowNodes and wire them together.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Task Flow"))
class TASKFLOWGRAPH_API UTaskFlowAsset : public UObject
{
	GENERATED_BODY()

public:
	UTaskFlowAsset();

	/** Graph data. */
	UPROPERTY()
	TObjectPtr<UTaskFlowGraph> Graph;

	/** Optional blackboard data; nodes/actions read and write its values at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	TObjectPtr<UTaskFlowBlackboard> Blackboard;

	/** Task type: Personal / World. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	ETaskFlowTaskType TaskType = ETaskFlowTaskType::World;

	UTaskFlowGraph* GetGraph() const { return Graph; }

	/** Entry node. */
	UTaskFlowNode_Start* GetStartNode() const;

	/** All nodes in the graph. */
	void GetAllNodes(TArray<UTaskFlowNode*>& OutNodes) const;

#if WITH_EDITOR
	/** Ensure the graph exists; create it with default Start / End nodes when empty. */
	UTaskFlowGraph* CreateOrLoadGraph();
#endif
};
