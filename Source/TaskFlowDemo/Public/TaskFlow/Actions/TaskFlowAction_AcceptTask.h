#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowAction.h"
#include "TaskFlowSystem.h"
#include "TaskFlowActorReference.h"
#include "TaskFlowAction_AcceptTask.generated.h"

/** Action that completes when the player reaches (enters) a TaskFlowVolume. */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "AcceptTask"))
class TASKFLOWDEMO_API UTaskFlowAction_AcceptTask : public UTaskFlowAction
{
	GENERATED_BODY()

public:
	UTaskFlowAction_AcceptTask();

	virtual FText GetActionName() const override;
	virtual void Execute_Implementation(UTaskFlowNode_Objective* OwnerNode) override;

	/** Override Finish to unbind from OnTaskFlowEvent. */
	virtual void Finish() override;

	/** Flow ended: stop listening for volume events so this action cannot complete after the flow has finished. */
	virtual void OnFlowEnded() override;

	/** Event name that means "reached the volume"; must match what the volume broadcasts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AcceptTask")
	int32 TaskId = 0;

private:
	/**
	 * The exact System whose OnTaskFlowEvent this action bound to. Cached at bind time so unbinding
	 * never depends on resolving the Runner/World again (the Runner is already detached when Finish() runs).
	 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UTaskFlowSystem> BoundSystem;
};
