#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowAsset.h"
#include "UObject/SoftObjectPtr.h"
#include "TaskFlowNode_SubFlow.generated.h"

class UTaskFlowRunner;

/**
 * Sub-flow node: runs another TaskFlowAsset as a child flow and waits for it to finish before
 * advancing the parent flow. It is an asynchronous node (it does not call Super::Execute).
 * It is an asynchronous node (Execute does not call Super).
 *
 * Guards: a null asset is treated as a no-op; self-reference and assets already running in this
 * World are rejected to avoid loops and the shared per-asset runtime state being overwritten.
 * and the shared runtime state on the asset (NodeState / ActiveRunner) being overwritten by multiple Runners.
 *
 * The child inherits the parent's context; the parent's node is resumed from OnFinished
 * (which also fires when the child is stopped), and OnFlowEnded stops the child to avoid orphan flows.
 * OnFlowEnded stops the child flow when the parent ends, so no orphan flow is left behind.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Sub Flow", TaskFlowCategory = "Flow"))
class TASKFLOWGRAPH_API UTaskFlowNode_SubFlow : public UTaskFlowNode
{
	GENERATED_BODY()

public:
	//~ Begin UEdGraphNode Interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface

#if WITH_EDITOR
	//~ Begin UObject Interface
	/** Refresh the node widget immediately after SubFlowAsset changes in the Details panel (the title is generated from it). */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;

	virtual void OnFlowEnded() override;

	/** TaskFlowAsset to run as a child flow (soft reference; resolved at execution time). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sub Flow")
	TSoftObjectPtr<UTaskFlowAsset> SubFlowAsset;

private:
	/** Called when the child Runner finishes (naturally, reached End, or was stopped). */
	UFUNCTION()
	void HandleSubFlowFinished(UTaskFlowRunner* FinishedRunner);

	/** Strong reference to keep the child Runner alive until its OnFinished arrives. */
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowRunner> SubRunner;

	/** Set once the parent flow ends; blocks late callbacks from advancing the flow. */
	bool bFlowEnded = false;
};
