#include "Nodes/TaskFlowNode_Gate.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "TaskFlowRunner.h"
#include "TaskFlowTypes.h"

#if WITH_EDITOR
#include "Framework/Commands/UIAction.h"
#include "Textures/SlateIcon.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"
#endif

#define LOCTEXT_NAMESPACE "TaskFlowNode_Gate"

namespace
{
	constexpr int32 MinGateInputs = 2;
	constexpr int32 MaxGateInputs = 16;

	FName MakeInputPinName(int32 Index)
	{
		return FName(*FString::Printf(TEXT("In_%d"), Index));
	}

	FText MakeInputPinLabel(int32 Index)
	{
		return FText::FromString(FString::Printf(TEXT("In %d"), Index));
	}
}

UTaskFlowNode_Gate::UTaskFlowNode_Gate()
{
}

void UTaskFlowNode_Gate::AllocateDefaultPins()
{
	const int32 Count = FMath::Clamp(InputCount, MinGateInputs, MaxGateInputs);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		UEdGraphPin* Pin = CreatePin(EGPD_Input, TaskFlowConstants::PinCategory, MakeInputPinName(Index));
		Pin->PinFriendlyName = MakeInputPinLabel(Index);
	}

	UEdGraphPin* OutPin = CreatePin(EGPD_Output, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Output);
	OutPin->PinFriendlyName = LOCTEXT("Gate_OutputPin", "Out");
}

void UTaskFlowNode_Gate::SyncInputPins()
{
	const int32 Desired = FMath::Clamp(InputCount, MinGateInputs, MaxGateInputs);
	InputCount = Desired;

	int32 Current = 0;
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin != nullptr && Pin->Direction == EGPD_Input)
		{
			++Current;
		}
	}

	if (Desired > Current)
	{
		for (int32 Index = Current; Index < Desired; ++Index)
		{
			UEdGraphPin* Pin = CreatePin(EGPD_Input, TaskFlowConstants::PinCategory, MakeInputPinName(Index));
			Pin->PinFriendlyName = MakeInputPinLabel(Index);
		}
	}
	else if (Desired < Current)
	{
		// Remove from the end so the remaining pins and their connections stay intact.
		// RemovePinAt breaks the links of the removed pin and notifies the graph.
		for (int32 Index = Current - 1; Index >= Desired; --Index)
		{
			RemovePinAt(Index, EGPD_Input);
		}
	}
}

#if WITH_EDITOR
void UTaskFlowNode_Gate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UTaskFlowNode_Gate, InputCount))
	{
		SyncInputPins();
		if (GetGraph() != nullptr)
		{
			GetGraph()->NotifyGraphChanged();
		}
	}
}
#endif

bool UTaskFlowNode_Gate::CanAddInput() const
{
	return InputCount < MaxGateInputs;
}

bool UTaskFlowNode_Gate::CanRemoveInput() const
{
	return InputCount > MinGateInputs;
}

void UTaskFlowNode_Gate::AddInput()
{
	if (!CanAddInput())
	{
		return;
	}

	Modify();
	++InputCount;
	SyncInputPins();

	if (GetGraph() != nullptr)
	{
		GetGraph()->NotifyGraphChanged();
	}
}

void UTaskFlowNode_Gate::RemoveLastInput()
{
	if (!CanRemoveInput())
	{
		return;
	}

	Modify();
	--InputCount;
	SyncInputPins();

	if (GetGraph() != nullptr)
	{
		GetGraph()->NotifyGraphChanged();
	}
}

#if WITH_EDITOR
void UTaskFlowNode_Gate::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	FToolMenuSection& Section = Menu->AddSection("TaskFlowGateInputs", LOCTEXT("GateInputsSection", "Inputs"));

	Section.AddMenuEntry(
		"AddInput",
		LOCTEXT("GateAddInput", "Add Input"),
		LOCTEXT("GateAddInputTooltip", "Adds a new input pin to this gate."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]() { const_cast<UTaskFlowNode_Gate*>(this)->AddInput(); }),
			FCanExecuteAction::CreateLambda([this]() { return CanAddInput(); })));

	Section.AddMenuEntry(
		"RemoveLastInput",
		LOCTEXT("GateRemoveLastInput", "Remove Last Input"),
		LOCTEXT("GateRemoveLastInputTooltip", "Removes the last input pin and its connections."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]() { const_cast<UTaskFlowNode_Gate*>(this)->RemoveLastInput(); }),
			FCanExecuteAction::CreateLambda([this]() { return CanRemoveInput(); })));
}
#endif

bool UTaskFlowNode_Gate::HasConnectedInput() const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin != nullptr && Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() > 0)
		{
			return true;
		}
	}

	return false;
}

TArray<UTaskFlowNode*> UTaskFlowNode_Gate::GetConnectedPredecessors() const
{
	TArray<UTaskFlowNode*> Result;
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin == nullptr || Pin->Direction != EGPD_Input)
		{
			continue;
		}

		for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
		{
			if (LinkedPin == nullptr)
			{
				continue;
			}

			if (UTaskFlowNode* Predecessor = Cast<UTaskFlowNode>(LinkedPin->GetOwningNode()))
			{
				Result.AddUnique(Predecessor);
			}
		}
	}

	return Result;
}

void UTaskFlowNode_Gate::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// Every incoming branch schedules this gate again. Check predecessor states instead of
	// counting executions, so branches that complete in the same batch are all accounted for.
	bShouldAdvance = !bTriggered && AreInputsSatisfied();
	if (bShouldAdvance)
	{
		bTriggered = true;
	}

	// Synchronous: the gate leaves ActiveNodes immediately so later branches can schedule it again.
	FinishExecution();
}

void UTaskFlowNode_Gate::GetNextNodes(TArray<UTaskFlowNode*>& OutNextNodes) const
{
	OutNextNodes.Reset();
	if (!bShouldAdvance)
	{
		return;
	}

	CollectNodesFromPin(GetOutputPin(), OutNextNodes);
}

void UTaskFlowNode_Gate::OnFlowEnded()
{
	bShouldAdvance = false;
	bTriggered = false;
	Super::OnFlowEnded();
}

bool UTaskFlowNode_Gate::ShouldMarkSucceededOnFinish() const
{
	// Each incoming branch finishes the gate synchronously, but it must stay yellow until the
	// gate actually fires Out. Once triggered, keep it green even if later branches schedule it again.
	return bTriggered;
}

bool UTaskFlowNode_Gate::AreInputsSatisfied() const
{
	return false;
}

bool UTaskFlowNode_And::AreInputsSatisfied() const
{
	// A gate with no connections passes through instead of deadlocking an unfinished graph.
	if (!HasConnectedInput())
	{
		return true;
	}

	const TArray<UTaskFlowNode*> Predecessors = GetConnectedPredecessors();
	for (const UTaskFlowNode* Predecessor : Predecessors)
	{
		if (Predecessor == nullptr || Predecessor->GetNodeState() != ETaskFlowNodeState::Succeeded)
		{
			return false;
		}
	}

	return Predecessors.Num() > 0;
}

bool UTaskFlowNode_Or::AreInputsSatisfied() const
{
	if (!HasConnectedInput())
	{
		return true;
	}

	const TArray<UTaskFlowNode*> Predecessors = GetConnectedPredecessors();
	for (const UTaskFlowNode* Predecessor : Predecessors)
	{
		if (Predecessor != nullptr && Predecessor->GetNodeState() == ETaskFlowNodeState::Succeeded)
		{
			return true;
		}
	}

	return false;
}

FText UTaskFlowNode_And::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("And_Title", "AND");
}

FText UTaskFlowNode_And::GetTooltipText() const
{
	return LOCTEXT("And_Tooltip", "Waits for all connected inputs to finish, then follows Out.");
}

FText UTaskFlowNode_Or::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Or_Title", "OR");
}

FText UTaskFlowNode_Or::GetTooltipText() const
{
	return LOCTEXT("Or_Tooltip", "Follows Out as soon as any connected input finishes (once per run).");
}

#undef LOCTEXT_NAMESPACE
