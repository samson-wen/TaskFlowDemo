#include "TaskFlowGraphNodeFactory.h"

#include "EdGraph/EdGraph.h"
#include "GraphEditorSettings.h"
#include "Layout/Margin.h"
#include "Nodes/TaskFlowNode_Gate.h"
#include "Nodes/TaskFlowNode_Random.h"
#include "ScopedTransaction.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "TaskFlowGraphNodeFactory"

void SGraphNode_TaskFlowGate::Construct(const FArguments& InArgs)
{
	SGraphNodeDefault::Construct(SGraphNodeDefault::FArguments().GraphNodeObj(InArgs._GraphNodeObj));
}

void SGraphNode_TaskFlowGate::CreateInputSideAddButton(TSharedPtr<SVerticalBox> InputBox)
{
	TSharedRef<SWidget> AddPinButton = AddPinButtonContent(
		LOCTEXT("GateAddPinButton", "Add Input"),
		LOCTEXT("GateAddPinButtonTooltip", "Adds another input pin to this gate."),
		false);

	FMargin AddPinPadding = Settings->GetInputPinPadding();
	AddPinPadding.Top += 6.0f;

	InputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(AddPinPadding)
		[
			AddPinButton
		];
}

EVisibility SGraphNode_TaskFlowGate::IsAddPinButtonVisible() const
{
	const UTaskFlowNode_Gate* Gate = Cast<UTaskFlowNode_Gate>(GraphNode);
	return (Gate != nullptr && Gate->CanAddInput()) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SGraphNode_TaskFlowGate::OnAddPin()
{
	if (UTaskFlowNode_Gate* Gate = Cast<UTaskFlowNode_Gate>(GraphNode))
	{
		FScopedTransaction Transaction(LOCTEXT("GateAddInputTransaction", "Add Gate Input"));
		Gate->AddInput();
		UpdateGraphNode();
	}

	return FReply::Handled();
}

void SGraphNode_TaskFlowRandom::Construct(const FArguments& InArgs)
{
	SGraphNodeDefault::Construct(SGraphNodeDefault::FArguments().GraphNodeObj(InArgs._GraphNodeObj));
}

void SGraphNode_TaskFlowRandom::CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox)
{
	TSharedRef<SWidget> AddPinButton = AddPinButtonContent(
		LOCTEXT("RandomAddPinButton", "Add Output"),
		LOCTEXT("RandomAddPinButtonTooltip", "Adds another weighted output pin."),
		true);

	FMargin AddPinPadding = Settings->GetOutputPinPadding();
	AddPinPadding.Top += 6.0f;

	OutputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.Padding(AddPinPadding)
		[
			AddPinButton
		];
}

EVisibility SGraphNode_TaskFlowRandom::IsAddPinButtonVisible() const
{
	const UTaskFlowNode_Random* RandomNode = Cast<UTaskFlowNode_Random>(GraphNode);
	return (RandomNode != nullptr && RandomNode->CanAddOutput()) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SGraphNode_TaskFlowRandom::OnAddPin()
{
	if (UTaskFlowNode_Random* RandomNode = Cast<UTaskFlowNode_Random>(GraphNode))
	{
		FScopedTransaction Transaction(LOCTEXT("RandomAddOutputTransaction", "Add Random Output"));
		RandomNode->AddOutput();
		UpdateGraphNode();
	}

	return FReply::Handled();
}

TSharedPtr<SGraphNode> FTaskFlowGraphNodeFactory::CreateNode(UEdGraphNode* Node) const
{
	if (Node != nullptr && Node->IsA<UTaskFlowNode_Random>())
	{
		return SNew(SGraphNode_TaskFlowRandom).GraphNodeObj(Node);
	}

	if (Node != nullptr && Node->IsA<UTaskFlowNode_Gate>())
	{
		return SNew(SGraphNode_TaskFlowGate).GraphNodeObj(Node);
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
