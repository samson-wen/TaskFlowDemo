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
	if (Graph.Get() == nullptr)
	{
		Graph = NewObject<UTaskFlowGraph>(this, UTaskFlowGraph::StaticClass(), TEXT("TaskFlowGraph"), RF_Transactional);
		Graph->CreateDefaultNodes();
	}

	return Graph;
}
#endif
