#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowObjective_KillTarget.generated.h"

class UTaskFlowRunner;

/**
 * Specialized Objective node that completes when the required number of enemies die.
 *
 * Handles EVENT_ENEMY_DEATH directly via OnTaskFlowEventReceived; no Action needs to be added.
 * It overrides ShouldListenToTaskFlowEvents() to opt into the (otherwise disabled) global event subscription.
 * The base class owns the event binding/unbinding (cached System) and the flow-ended guard.
 * The base class does not subscribe to global events by default; this node overrides ShouldListenToTaskFlowEvents() to opt in;
 * binding / unbinding the event delegate (cached System) and the flow-ended guard are handled by the base class.
 *
 * Set MaxProgress (inherited >= 1) to require multiple kills. AIID = 0 matches any enemy.
 */
UCLASS(Blueprintable, meta = (DisplayName = "KillTarget Objective", TaskFlowCategory = "Task"))
class TASKFLOWDEMO_API UTaskFlowObjective_KillTarget : public UTaskFlowNode_Objective
{
	GENERATED_BODY()

public:
	UTaskFlowObjective_KillTarget();

	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;

protected:
	virtual void OnTaskFlowEventReceived(const FTaskFlowEvent& Event) override;

	/** This Objective requires OnTaskFlowEvent to count kills, so it always opts in. */
	virtual bool ShouldListenToTaskFlowEvents() const override;

public:
	/** Only count deaths of the enemy with this AIID; 0 means any enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kill Target")
	int64 AIID = 0;
};
