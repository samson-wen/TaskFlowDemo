#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_Random.generated.h"

class UEdGraphPin;
class UTaskFlowRunner;

/** Per-output weight configuration for the Random node. One entry per output pin. */
USTRUCT(BlueprintType)
struct FTaskFlowRandomOutput
{
	GENERATED_BODY()

	/** Relative chance of this output being selected. Negative values are treated as 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random", meta = (ClampMin = "0.0"))
	float Weight = 1.f;
};

/**
 * Random branch node: one input, several weighted outputs.
 * On execution it rolls a weighted random output and follows only that pin's successors.
 * The number of outputs is controlled by the node's + button / context menu; their weights are
 * edited in the Details panel (the array size is fixed there so pin count stays in sync).
 */
UCLASS(Blueprintable, meta = (DisplayName = "Random", TaskFlowCategory = "Flow"))
class TASKFLOWGRAPH_API UTaskFlowNode_Random : public UTaskFlowNode
{
	GENERATED_BODY()

public:
	UTaskFlowNode_Random();

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
#endif
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;
	virtual void GetNextNodes(TArray<UTaskFlowNode*>& OutNextNodes) const override;
	virtual void OnFlowEnded() override;

	/**
	 * One entry per output pin. EditFixedSize keeps the array length controlled by the node UI;
	 * only the weights are editable in the Details panel.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category = "Random")
	TArray<FTaskFlowRandomOutput> Outputs;

	/** Add one weighted output pin. */
	void AddOutput();

	/** Remove the last output pin and its connections. */
	void RemoveLastOutput();

	bool CanAddOutput() const;
	bool CanRemoveOutput() const;

private:
	/** Output pin at the given index among output pins only. */
	UEdGraphPin* GetOutputPinByIndex(int32 Index) const;

	/** Create / remove output pins so they match Outputs, without rebuilding the node. */
	void SyncOutputPins();

	/** Weighted random pick; uniform when all weights are zero. */
	int32 PickOutputIndex() const;

	/** Friendly name shown on an output pin, including its weight. */
	FText MakeOutputPinLabel(int32 Index) const;

	/** Index chosen by the most recent execution. */
	int32 SelectedOutputIndex = INDEX_NONE;
};
