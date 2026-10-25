#include "TaskFlowGraphSchema.h"

#include "EdGraph/EdGraphPin.h"
#include "Math/Vector2D.h"
#include "Nodes/TaskFlowNode.h"
#include "Nodes/TaskFlowNode_End.h"
#include "Nodes/TaskFlowNode_Start.h"
#include "TaskFlowGraph.h"
#include "TaskFlowTypes.h"
#include "UObject/UObjectHash.h"

#define LOCTEXT_NAMESPACE "TaskFlowGraphSchema"

namespace TaskFlowSchemaUtils
{
	/** 递归判断从 From 出发是否可以到达 To */
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

	/** 在指定位置创建一个节点 */
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

	// 图中已存在的唯一节点（例如 Start），不再提供创建入口
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

	// 收集所有可实例化的 UTaskFlowNode 子类，自动生成菜单项
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
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError_Invalid", "引脚无效"));
	}

	if (A->GetOwningNode() == B->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError_SameNode", "不能连接到自己"));
	}

	if (A->Direction == B->Direction)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError_SameDirection", "只能从输出引脚连接到输入引脚"));
	}

	const UEdGraphPin* OutputPin = (A->Direction == EGPD_Output) ? A : B;
	const UEdGraphPin* InputPin = (A->Direction == EGPD_Output) ? B : A;

	UEdGraphNode* OutputNode = OutputPin->GetOwningNode();
	UEdGraphNode* InputNode = InputPin->GetOwningNode();

	// 不允许成环
	TSet<const UEdGraphNode*> Visited;
	if (TaskFlowSchemaUtils::CanReach(InputNode, OutputNode, Visited))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError_Cycle", "不允许连成一个环"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, FText::GetEmpty());
}

void UTaskFlowGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	TaskFlowSchemaUtils::SpawnNode<UTaskFlowNode_Start>(Graph, FVector2f(0.f, 0.f));
	TaskFlowSchemaUtils::SpawnNode<UTaskFlowNode_End>(Graph, FVector2f(400.f, 0.f));
}

void UTaskFlowGraphSchema::GetGraphDisplayInformation(const UEdGraph& Graph, FGraphDisplayInfo& DisplayInfo) const
{
	DisplayInfo.PlainName = LOCTEXT("TaskFlowGraph_PlainName", "任务流程");
	DisplayInfo.DisplayName = DisplayInfo.PlainName;
	DisplayInfo.Tooltip = LOCTEXT("TaskFlowGraph_Tooltip", "任务流程图：从 Start 节点开始，依次执行连线上后面的节点");
}

FLinearColor UTaskFlowGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FLinearColor::White;
}

bool UTaskFlowGraphSchema::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
