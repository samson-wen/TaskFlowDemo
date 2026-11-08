#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowAction.h"
#include "TaskFlowSystem.h"
#include "TaskFlowAction_ReachVolume.generated.h"

/** Action that completes when the player reaches (enters) a TaskFlowVolume. */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "ReachVolume"))
class TASKFLOWDEMO_API UTaskFlowAction_ReachVolume : public UTaskFlowAction
{
	GENERATED_BODY()

public:
	UTaskFlowAction_ReachVolume();

	virtual FText GetActionName() const override;
	virtual void Execute_Implementation(UTaskFlowNode_Objective* OwnerNode) override;

	/** Called by the system's OnTaskFlowEvent while this action is awaiting a volume-enter event. */
	UFUNCTION()
	void OnTaskFlowEventReceived(const FTaskFlowEvent& Event);

	/** Override Finish to unbind from OnTaskFlowEvent. */
	virtual void Finish() override;

	/** Flow ended: stop listening for volume events so this action cannot complete after the flow has finished. */
	virtual void OnFlowEnded() override;

	/** Event name that means "reached the volume"; must match what the volume broadcasts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReachVolume")
	int32 VolumeID = 0;

	/** Only finish when the entered volume's tag matches this value; empty means any volume. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReachVolume")
	FName VolumeTag = NAME_None;

private:
	/** Unbind this action from the system's global event delegate, using the cached System. */
	void UnbindFromTaskFlowEvent();

	/**
	 * The exact System whose OnTaskFlowEvent this action bound to. Cached at bind time so unbinding
	 * never depends on resolving the Runner/World again (the Runner is already detached when Finish() runs).
	 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UTaskFlowSystem> BoundSystem;
};
