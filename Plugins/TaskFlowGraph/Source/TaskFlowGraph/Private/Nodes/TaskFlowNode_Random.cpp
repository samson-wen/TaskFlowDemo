#include "Nodes/TaskFlowNode_Random.h"

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

#define LOCTEXT_NAMESPACE "TaskFlowNode_Random"

namespace
{
	constexpr int32 MinRandomOutputs = 1;
	constexpr int32 MaxRandomOutputs = 16;

	FName MakeOutputPinName(int32 Index)
	{
		return FName(*FString::Printf(TEXT("Out_%d"), Index));
	}
}

UTaskFlowNode_Random::UTaskFlowNode_Random()
{
	Outputs.Add(FTaskFlowRandomOutput());
	Outputs.Add(FTaskFlowRandomOutput());
}

FText UTaskFlowNode_Random::MakeOutputPinLabel(int32 Index) const
{
	const float Weight = Outputs.IsValidIndex(Index) ? Outputs[Index].Weight : 0.f;
	return FText::FromString(FString::Printf(TEXT("Out %d  (W: %.2f)"), Index, Weight));
}

void UTaskFlowNode_Random::AllocateDefaultPins()
{
	UEdGraphPin* InputPin = CreatePin(EGPD_Input, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Input);
	InputPin->PinFriendlyName = LOCTEXT("RandomInputPin", "In");

	const int32 Count = FMath::Clamp(Outputs.Num(), MinRandomOutputs, MaxRandomOutputs);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		UEdGraphPin* Pin = CreatePin(EGPD_Output, TaskFlowConstants::PinCategory, MakeOutputPinName(Index));
		Pin->PinFriendlyName = MakeOutputPinLabel(Index);
	}
}

void UTaskFlowNode_Random::SyncOutputPins()
{
	int32 Current = 0;
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin != nullptr && Pin->Direction == EGPD_Output)
		{
			++Current;
		}
	}

	const int32 Desired = FMath::Clamp(Outputs.Num(), MinRandomOutputs, MaxRandomOutputs);
	if (Desired > Current)
	{
		for (int32 Index = Current; Index < Desired; ++Index)
		{
			UEdGraphPin* Pin = CreatePin(EGPD_Output, TaskFlowConstants::PinCategory, MakeOutputPinName(Index));
			Pin->PinFriendlyName = MakeOutputPinLabel(Index);
		}
	}
	else if (Desired < Current)
	{
		// Remove from the end so the remaining pins and their connections stay intact.
		for (int32 Index = Current - 1; Index >= Desired; --Index)
		{
			RemovePinAt(Index, EGPD_Output);
		}
	}
}

bool UTaskFlowNode_Random::CanAddOutput() const
{
	return Outputs.Num() < MaxRandomOutputs;
}

bool UTaskFlowNode_Random::CanRemoveOutput() const
{
	return Outputs.Num() > MinRandomOutputs;
}

void UTaskFlowNode_Random::AddOutput()
{
	if (!CanAddOutput())
	{
		return;
	}

	Modify();
	Outputs.Add(FTaskFlowRandomOutput());
	SyncOutputPins();

	if (GetGraph() != nullptr)
	{
		GetGraph()->NotifyGraphChanged();
	}
}

void UTaskFlowNode_Random::RemoveLastOutput()
{
	if (!CanRemoveOutput())
	{
		return;
	}

	Modify();
	Outputs.RemoveAt(Outputs.Num() - 1);
	SyncOutputPins();

	if (GetGraph() != nullptr)
	{
		GetGraph()->NotifyGraphChanged();
	}
}

#if WITH_EDITOR
void UTaskFlowNode_Random::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Keep pin labels (which show the weights) and pin count in sync with the Outputs array.
	SyncOutputPins();
	int32 OutputIndex = 0;
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin != nullptr && Pin->Direction == EGPD_Output)
		{
			Pin->PinFriendlyName = MakeOutputPinLabel(OutputIndex);
			++OutputIndex;
		}
	}

	if (GetGraph() != nullptr)
	{
		GetGraph()->NotifyGraphChanged();
	}
}

void UTaskFlowNode_Random::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	FToolMenuSection& Section = Menu->AddSection("TaskFlowRandomOutputs", LOCTEXT("RandomOutputsSection", "Outputs"));

	Section.AddMenuEntry(
		"AddOutput",
		LOCTEXT("RandomAddOutput", "Add Output"),
		LOCTEXT("RandomAddOutputTooltip", "Adds a new weighted output pin."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]() { const_cast<UTaskFlowNode_Random*>(this)->AddOutput(); }),
			FCanExecuteAction::CreateLambda([this]() { return CanAddOutput(); })));

	Section.AddMenuEntry(
		"RemoveLastOutput",
		LOCTEXT("RandomRemoveLastOutput", "Remove Last Output"),
		LOCTEXT("RandomRemoveLastOutputTooltip", "Removes the last output pin and its connections."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]() { const_cast<UTaskFlowNode_Random*>(this)->RemoveLastOutput(); }),
			FCanExecuteAction::CreateLambda([this]() { return CanRemoveOutput(); })));
}
#endif

int32 UTaskFlowNode_Random::PickOutputIndex() const
{
	const int32 Count = Outputs.Num();
	if (Count == 0)
	{
		return INDEX_NONE;
	}

	float TotalWeight = 0.f;
	for (const FTaskFlowRandomOutput& Output : Outputs)
	{
		TotalWeight += FMath::Max(0.f, Output.Weight);
	}

	// All weights zero: fall back to a uniform pick instead of always taking the first output.
	if (TotalWeight <= 0.f)
	{
		return FMath::RandRange(0, Count - 1);
	}

	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		Roll -= FMath::Max(0.f, Outputs[Index].Weight);
		if (Roll <= 0.f)
		{
			return Index;
		}
	}

	// Guard against floating point rounding leaving a tiny remainder.
	return Count - 1;
}

void UTaskFlowNode_Random::Execute_Implementation(UTaskFlowRunner* Runner)
{
	SelectedOutputIndex = PickOutputIndex();
	FinishExecution();
}

void UTaskFlowNode_Random::GetNextNodes(TArray<UTaskFlowNode*>& OutNextNodes) const
{
	OutNextNodes.Reset();
	if (SelectedOutputIndex == INDEX_NONE)
	{
		return;
	}

	CollectNodesFromPin(GetOutputPinByIndex(SelectedOutputIndex), OutNextNodes);
}

void UTaskFlowNode_Random::OnFlowEnded()
{
	SelectedOutputIndex = INDEX_NONE;
	Super::OnFlowEnded();
}

UEdGraphPin* UTaskFlowNode_Random::GetOutputPinByIndex(int32 Index) const
{
	int32 Found = 0;
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin != nullptr && Pin->Direction == EGPD_Output)
		{
			if (Found == Index)
			{
				return Pin;
			}
			++Found;
		}
	}

	return nullptr;
}

FText UTaskFlowNode_Random::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Random_Title", "Random");
}

FText UTaskFlowNode_Random::GetTooltipText() const
{
	return LOCTEXT("Random_Tooltip", "Follows exactly one output, chosen by weighted random.");
}

#undef LOCTEXT_NAMESPACE
