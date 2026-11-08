#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "SGraphNodeDefault.h"

class SVerticalBox;
class UEdGraphNode;
class UTaskFlowNode_Gate;
class UTaskFlowNode_Random;

/**
 * Node widget for AND / OR gates. Adds a "+" button on the input side that appends an input pin,
 * reusing the standard SGraphNode add-pin button hooks.
 */
class SGraphNode_TaskFlowGate : public SGraphNodeDefault
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_TaskFlowGate)
		: _GraphNodeObj(nullptr)
	{}

		SLATE_ARGUMENT(UEdGraphNode*, GraphNodeObj)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

protected:
	//~ Begin SGraphNode Interface
	virtual void CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox) override;
	virtual EVisibility IsAddPinButtonVisible() const override;
	virtual FReply OnAddPin() override;
	//~ End SGraphNode Interface
};

/**
 * Node widget for the Random node. Adds a "+" button on the output side that appends a weighted output.
 */
class SGraphNode_TaskFlowRandom : public SGraphNodeDefault
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_TaskFlowRandom)
		: _GraphNodeObj(nullptr)
	{}

		SLATE_ARGUMENT(UEdGraphNode*, GraphNodeObj)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

protected:
	//~ Begin SGraphNode Interface
	virtual void CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox) override;
	virtual EVisibility IsAddPinButtonVisible() const override;
	virtual FReply OnAddPin() override;
	//~ End SGraphNode Interface
};

/** Visual-node factory that installs the custom widgets for gate and random nodes. */
struct FTaskFlowGraphNodeFactory : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* Node) const override;
};
