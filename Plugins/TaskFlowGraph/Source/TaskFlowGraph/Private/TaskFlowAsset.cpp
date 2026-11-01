#include "TaskFlowAsset.h"

#include "Nodes/TaskFlowNode.h"
#include "Nodes/TaskFlowNode_Start.h"
#include "TaskFlowGraph.h"

UTaskFlowAsset::UTaskFlowAsset()
{
}

UTaskFlowNode_Start* UTaskFlowAsset::GetStartNode() const
{
	return Graph.Get() != nullptr ? Graph->GetStartNode() : nullptr;
}

void UTaskFlowAsset::GetAllNodes(TArray<UTaskFlowNode*>& OutNodes) const
{
	if (Graph.Get() != nullptr)
	{
		Graph->GetAllNodes(OutNodes);
	}
	else
	{
		OutNodes.Reset();
	}
}

#if WITH_EDITOR
UTaskFlowGraph* UTaskFlowAsset::CreateOrLoadGraph()
{
	// [EN] Create the graph with default Start / End nodes if it does not exist yet.
	// [CN] 为空时创建并放置默认的 Start / End 节点
	if (Graph.Get() == nullptr)
	{
		Graph = NewObject<UTaskFlowGraph>(this, UTaskFlowGraph::StaticClass(), TEXT("TaskFlowGraph"), RF_Transactional);
		Graph->CreateDefaultNodes();
	}

	return Graph;
}
#endif
