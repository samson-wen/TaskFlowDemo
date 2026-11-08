#include "TaskFlowGraph.h"

#include "Nodes/TaskFlowNode.h"
#include "Nodes/TaskFlowNode_Start.h"
#include "TaskFlowAsset.h"
#include "TaskFlowGraphSchema.h"

UTaskFlowGraph::UTaskFlowGraph()
{
	// Bind this graph to the TaskFlow schema so the editor knows how to render and connect nodes.
	Schema = UTaskFlowGraphSchema::StaticClass();
}

UTaskFlowAsset* UTaskFlowGraph::GetTaskFlowAsset() const
{
	return Cast<UTaskFlowAsset>(GetOuter());
}

UTaskFlowNode_Start* UTaskFlowGraph::GetStartNode() const
{
	for (UEdGraphNode* Node : Nodes)
	{
		if (UTaskFlowNode_Start* StartNode = Cast<UTaskFlowNode_Start>(Node))
		{
			return StartNode;
		}
	}

	return nullptr;
}

void UTaskFlowGraph::GetAllNodes(TArray<UTaskFlowNode*>& OutNodes) const
{
	OutNodes.Reset();

	for (UEdGraphNode* Node : Nodes)
	{
		if (UTaskFlowNode* TaskNode = Cast<UTaskFlowNode>(Node))
		{
			OutNodes.Add(TaskNode);
		}
	}
}

#if WITH_EDITOR
void UTaskFlowGraph::CreateDefaultNodes()
{
	// Do nothing if the graph already has nodes; otherwise ask the schema to create default nodes.
	if (Nodes.Num() > 0)
	{
		return;
	}

	if (const UEdGraphSchema* GraphSchema = GetSchema())
	{
		GraphSchema->CreateDefaultNodesForGraph(*this);
	}
}
#endif
