#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "TaskFlowGraphSchema.generated.h"

/**
 * [EN] Schema for the task flow graph:
 * [CN] 任务流图的 Schema：
 * [EN] - Lists all creatable UTaskFlowNode subclasses in the right-click menu.
 * [CN] - 右键菜单里列出所有可被创建的 UTaskFlowNode 子类
 * [EN] - Enforces connection rules (output -> input, no cycles allowed).
 * [CN] - 约束连线规则（输出 -> 输入，不允许成环）
 */
UCLASS(MinimalAPI)
class UTaskFlowGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	//~ Begin UEdGraphSchema Interface
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual void GetGraphDisplayInformation(const UEdGraph& Graph, FGraphDisplayInfo& DisplayInfo) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
	virtual bool ShouldHidePinDefaultValue(UEdGraphPin* Pin) const override;
	virtual FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const override;
	//~ End UEdGraphSchema Interface
};
