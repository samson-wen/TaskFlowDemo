#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowObjective_AcceptTask.generated.h"

class UTaskFlowRunner;

/**
 * Specialized Objective node that waits until a task is accepted and then completes.
 *
 * It listens to EVENT_TASK_ACCEPTED on UTaskFlowSystem::OnTaskFlowEvent; the base class owns the
 * binding / unbinding and the flow-ended guard. The event is normally emitted from the task UI or
 * task component via UGameTaskFlowStatics::BroadcastTaskAccepted after the task has been added to
 * the player's accepted-task list.
 *
 * TaskID matches FGameTask::TaskID in the task DataTable; 0 matches any task. MaxProgress is
 * clamped to >= 1 on execution, so the node stays executing until a matching accept event arrives
 * (and until asynchronous Actions finish, if any are configured).
 *
 * This node only observes acceptance - it does not write accepted-task state and does not start the
 * task's TaskFlowAsset. Ownership of that state belongs to the player / task component.
 */
UCLASS(Blueprintable, meta = (DisplayName = "AcceptTask Objective", TaskFlowCategory = "Task"))
class TASKFLOWDEMO_API UTaskFlowObjective_AcceptTask : public UTaskFlowNode_Objective
{
	GENERATED_BODY()

public:
	UTaskFlowObjective_AcceptTask();

	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;

protected:
	void AcceptTask(APlayerState* PS, int32 InTaskID);

public:
	/** Task identifier to wait for; 0 matches any accepted task. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accept Task")
	int32 TaskID = 0;
};
