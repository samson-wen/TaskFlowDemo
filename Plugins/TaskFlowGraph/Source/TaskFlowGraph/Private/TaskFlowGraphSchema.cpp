#include "TaskFlowGraphSchema.h"

#include "EdGraph/EdGraphPin.h"
#include "Math/Vector2D.h"
#include "Nodes/TaskFlowNode.h"
#include "Nodes/TaskFlowNode_End.h"
#include "Nodes/TaskFlowNode_Start.h"
#include "TaskFlowGraph.h"
#include "TaskFlowTypes.h"
#include "UObject/UObjectHash.h"

#if WITH_EDITOR
#include "TaskFlowConnectionDrawingPolicy.h"
#endif

#define LOCTEXT_NAMESPACE "TaskFlowGraphSchema"

namespace TaskFlowSchemaUtils
{
	/** Recursively check whether To can be reached starting from From. */
	bool CanReach(const UEdGraphNode* From, const UEdGraphNode* To, TSet<const UEdGraphNode*>& Visited)
	{
		if (From == nullptr || To == nullptr)
		{
			return false;
		}

		if (From == To)
		{
			return true;
		}

		bool bAlreadyVisited = false;
		Visited.Add(From, &bAlreadyVisited);
		if (bAlreadyVisited)
		{
			return false;
		}

		for (const UEdGraphPin* Pin : From->Pins)
		{
			if (Pin == nullptr || Pin->Direction != EGPD_Output)
			{
				continue;
			}

			for (const UEdGraphPin* LinkedPin : Pin->LinkedTo)
			{
				if (LinkedPin == nullptr)
				{
					continue;
				}

				if (CanReach(LinkedPin->GetOwningNode(), To, Visited))
				{
					return true;
				}
			}
		}

		return false;
	}

	/** Create a node of the given type at the specified location. */
	template <typename NodeType>
	NodeType* SpawnNode(UEdGraph& Graph, const FVector2f& Location)
	{
		NodeType* Template = NewObject<NodeType>();
		return FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<NodeType>(&Graph, Template, Location);
	}
}

void UTaskFlowGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	const UTaskFlowGraph* TaskFlowGraph = Cast<UTaskFlowGraph>(ContextMenuBuilder.CurrentGraph);

	// Unique nodes already present in the graph (e.g. Start) are not offered for creation again.
	TSet<UClass*> ExistingUniqueClasses;
	if (TaskFlowGraph != nullptr)
	{
		TArray<UTaskFlowNode*> ExistingNodes;
		TaskFlowGraph->GetAllNodes(ExistingNodes);
		for (const UTaskFlowNode* Node : ExistingNodes)
		{
			if (Node != nullptr && Node->IsUniqueNode())
			{
				ExistingUniqueClasses.Add(Node->GetClass());
			}
		}
	}

	// Collect all instantiable UTaskFlowNode subclasses and generate menu entries automatically.
	TArray<UClass*> NodeClasses;
	GetDerivedClasses(UTaskFlowNode::StaticClass(), NodeClasses, true);

	for (UClass* NodeClass : NodeClasses)
	{
		if (NodeClass == nullptr
			|| NodeClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)
			|| NodeClass->GetName().StartsWith(TEXT("SKEL_"))
			|| NodeClass->GetName().StartsWith(TEXT("REINST_")))
		{
			continue;
		}

		if (ExistingUniqueClasses.Contains(NodeClass))
		{
			continue;
		}

		UTaskFlowNode* Template = NewObject<UTaskFlowNode>(ContextMenuBuilder.OwnerOfTemporaries, NodeClass);

		const FText Category = Template->GetNodeCategory();
		const FText MenuDesc = Template->GetNodeTitle(ENodeTitleType::ListView);
		const FText Tooltip = Template->GetTooltipText();

		TSharedPtr<FEdGraphSchemaAction_NewNode> NewNodeAction = MakeShared<FEdGraphSchemaAction_NewNode>(Category, MenuDesc, Tooltip, 0);
		NewNodeAction->NodeTemplate = Template;

		ContextMenuBuilder.AddAction(NewNodeAction);
	}
}

const FPinConnectionResponse UTaskFlowGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (A == nullptr || B == nullptr)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError_Invalid", "Invalid pin"));
	}

	if (A->GetOwningNode() == B->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError_SameNode", "Cannot connect a node to itself"));
	}

	if (A->Direction == B->Direction)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError_SameDirection", "Connections must go from an output pin to an input pin"));
	}

	const UEdGraphPin* OutputPin = (A->Direction == EGPD_Output) ? A : B;
	const UEdGraphPin* InputPin = (A->Direction == EGPD_Output) ? B : A;

	UEdGraphNode* OutputNode = OutputPin->GetOwningNode();
	UEdGraphNode* InputNode = InputPin->GetOwningNode();

	// Disallow creating cycles.
	TSet<const UEdGraphNode*> Visited;
	if (TaskFlowSchemaUtils::CanReach(InputNode, OutputNode, Visited))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError_Cycle", "Connections cannot form a cycle"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, FText::GetEmpty());
}

void UTaskFlowGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	// Spawn the default Start node at (0,0) and the End node at (400,0).
	TaskFlowSchemaUtils::SpawnNode<UTaskFlowNode_Start>(Graph, FVector2f(0.f, 0.f));
	TaskFlowSchemaUtils::SpawnNode<UTaskFlowNode_End>(Graph, FVector2f(400.f, 0.f));
}

void UTaskFlowGraphSchema::GetGraphDisplayInformation(const UEdGraph& Graph, FGraphDisplayInfo& DisplayInfo) const
{
	DisplayInfo.PlainName = LOCTEXT("TaskFlowGraph_PlainName", "Task Flow");
	DisplayInfo.DisplayName = DisplayInfo.PlainName;
	DisplayInfo.Tooltip = LOCTEXT("TaskFlowGraph_Tooltip", "Task Flow Graph: execution starts at the Start node, then follows the connected nodes in order");
}

FLinearColor UTaskFlowGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FLinearColor::White;
}

bool UTaskFlowGraphSchema::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const
{
	return true;
}

FConnectionDrawingPolicy* UTaskFlowGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
#if WITH_EDITOR
	// Use the custom policy so links can show an animated flow at runtime.
	return new FTaskFlowConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
#else
	return nullptr;
#endif
}

#undef LOCTEXT_NAMESPACE
