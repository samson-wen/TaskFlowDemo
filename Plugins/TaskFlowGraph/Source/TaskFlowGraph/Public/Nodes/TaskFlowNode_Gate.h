#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_Gate.generated.h"

class UEdGraphPin;
class UTaskFlowRunner;

/**
 * Base class for multi-input gate nodes (AND / OR).
 *
 * A gate has a configurable number of input pins and a single Out pin. Every incoming branch
 * schedules the gate again; it evaluates the runtime state of the connected predecessor nodes
 * instead of counting executions, so several branches arriving in the same batch are handled too.
 * The gate finishes synchronously and only follows Out when its condition is met (once per run).
 */
UCLASS(Abstract, Blueprintable, meta = (DisplayName = "Gate", TaskFlowCategory = "Flow"))
class TASKFLOWGRAPH_API UTaskFlowNode_Gate : public UTaskFlowNode
{
	GENERATED_BODY()

public:
	UTaskFlowNode_Gate();

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
#endif
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;
	virtual void GetNextNodes(TArray<UTaskFlowNode*>& OutNextNodes) const override;
	virtual void OnFlowEnded() override;
	virtual bool ShouldMarkSucceededOnFinish() const override;

	/** Number of input pins. Changing this in the Details panel adds / removes inputs dynamically. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate", meta = (ClampMin = "2", ClampMax = "16"))
	int32 InputCount = 2;

	/** Add one input pin (used by the node's + button and context menu). */
	void AddInput();

	/** Remove the last input pin and its connections. */
	void RemoveLastInput();

	bool CanAddInput() const;
	bool CanRemoveInput() const;

protected:
	/** Gate condition, overridden by AND / OR. The default never advances (kept non-pure so UHT can instantiate the class). */
	virtual bool AreInputsSatisfied() const;

	/** All distinct predecessor nodes connected to any input pin. */
	TArray<UTaskFlowNode*> GetConnectedPredecessors() const;

	/** Whether at least one input pin has a connection. */
	bool HasConnectedInput() const;

private:
	/** Add / remove input pins to match InputCount without rebuilding the node (existing links survive). */
	void SyncInputPins();

	/** Result of the most recent evaluation; decides whether GetNextNodes follows Out. */
	bool bShouldAdvance = false;

	/** Guards against following Out more than once per flow run. */
	bool bTriggered = false;
};

/**
 * AND gate: waits until every connected input branch has finished, then follows Out.
 * Unconnected input pins are ignored; a gate with no connections passes through immediately.
 */
UCLASS(Blueprintable, meta = (DisplayName = "AND", TaskFlowCategory = "Flow"))
class TASKFLOWGRAPH_API UTaskFlowNode_And : public UTaskFlowNode_Gate
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;

protected:
	virtual bool AreInputsSatisfied() const override;
};

/**
 * OR gate: follows Out as soon as any connected input branch finishes, and only once per run.
 */
UCLASS(Blueprintable, meta = (DisplayName = "OR", TaskFlowCategory = "Flow"))
class TASKFLOWGRAPH_API UTaskFlowNode_Or : public UTaskFlowNode_Gate
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;

protected:
	virtual bool AreInputsSatisfied() const override;
};
