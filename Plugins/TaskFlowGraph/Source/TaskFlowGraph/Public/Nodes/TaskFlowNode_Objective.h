#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowEvent.h"
#include "TaskFlowSystem.h"
#include "TaskFlowNode_Objective.generated.h"

class UTaskFlowAction;
class UTaskFlowRunner;

/**
 * Objective node: represents a goal / step in the task flow.
 * It inherits from UTaskFlowNode and can be created and wired directly in the TaskFlowGraph asset editor;
 * when triggered, it executes the actions in the Actions list in order, and advances to the next node once all are done.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Objective", TaskFlowCategory = "Task"))
class TASKFLOWGRAPH_API UTaskFlowNode_Objective : public UTaskFlowNode
{
	GENERATED_BODY()

public:
	UTaskFlowNode_Objective();

	//~ Begin UEdGraphNode Interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FName GetNodeName() const override;
	virtual void PostPasteNode() override;
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;

	/** Flow ended: unbind from OnTaskFlowEvent, abort all pending Actions and ignore late callbacks. */
	virtual void OnFlowEnded() override;

	/** Objective description; shown in the node tooltip and the Details panel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (MultiLine = true))
	FText ObjectiveDescription;

	/** Node name, must be unique within the same asset (empty means unnamed, not validated). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName NodeName;

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif

	/** List of actions to execute, in order. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", Instanced)
	TArray<TObjectPtr<UTaskFlowAction>> Actions;

	/** Maximum progress; 0 means no progress needed; >0 requires current progress to reach this value to complete. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (ClampMin = "0"))
	int32 MaxProgress = 0;

	/** Current progress (runtime); the Objective completes once it reaches MaxProgress. */
	UPROPERTY(BlueprintReadOnly, Category = "Objective", Transient)
	int32 CurrentProgress = 0;

	/**
	 * Whether this Objective subscribes to UTaskFlowSystem::OnTaskFlowEvent while executing.
	 * Disabled by default; enable it only when the node (or one of its Actions via ReceiveTaskFlowEvent) needs events.
	 * (via ReceiveTaskFlowEvent) needs to receive events.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Event")
	bool bListenToTaskFlowEvents = false;

	/**
	 * Enable/disable event listening at runtime. Takes effect immediately while the Objective is executing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective|Event")
	void SetTaskFlowEventListening(bool bEnable);

	/** Add to the current progress. */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void AddProgress(int32 Amount);

	/** Set the current progress. */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetProgress(int32 Value);

	/** Internal: callback when an action finishes, to continue the next action. */
	void OnActionFinished(UTaskFlowAction* Action);

	/** Get the TaskFlowSystem driving the current flow (via the owning Objective's Runner, falling back to the world subsystem). */
	UFUNCTION(BlueprintPure, Category = "Objective")
	UTaskFlowSystem* GetTaskFlowSystem() const;

protected:
	/**
	 * Hook invoked for every event received while this Objective is executing, BEFORE forwarding to Actions.
	 * Specialized Objective subclasses override this to handle events directly; the default does nothing.
	 * Only reached when listening is enabled (see ShouldListenToTaskFlowEvents). The base still handles
	 * flow-ended / non-executing guards and binding/unbinding.
	 * as well as binding / unbinding the event delegate; subclasses do not need to repeat any of this.
	 */
	virtual void OnTaskFlowEventReceived(const FTaskFlowEvent& Event);

	/**
	 * Decides whether this Objective subscribes to OnTaskFlowEvent when it starts executing.
	 * Base returns bListenToTaskFlowEvents (false by default); specialized subclasses may override to always listen.
	 * Specialized subclasses may override this to always listen (e.g. KillTarget advances on kill events).
	 */
	virtual bool ShouldListenToTaskFlowEvents() const;

private:
	/** Bind HandleTaskFlowEvent to the given System, using the cached System for later unbinding. */
	void BindToTaskFlowEvent(UTaskFlowSystem* System);
	/** Determine whether the current NodeName duplicates another Objective node in the graph. */
	bool IsNodeNameDuplicate() const;

	/** Continue executing the next action. */
	void ExecuteNextAction();

	/** When all actions are done and progress is met, call FinishExecution to complete the node. */
	void TryComplete();

	/** Unbind from OnTaskFlowEvent using the cached System. */
	void UnbindFromTaskFlowEvent();

	/** The exact System whose OnTaskFlowEvent this node bound to; cached at bind time. */
	UPROPERTY(Transient)
	TWeakObjectPtr<UTaskFlowSystem> BoundSystem;

	/** Index of the action currently executing (or waiting to finish). */
	int32 CurrentActionIndex = 0;

	/** Prevent recursion caused by Finish() inside synchronous actions. */
	bool bAdvancingActions = false;

	/** Whether all actions have been executed. */
	bool bActionsCompleted = false;

	/** Set once the flow ends; blocks late event / action callbacks from advancing this Objective. */
	bool bFlowEnded = false;

	/** NodeName before editing, used to roll back on duplication. */
	FName OldNodeName;

	/** Internal handler bound to UTaskFlowSystem::OnTaskFlowEvent; forwards events to unfinished Actions. */
	UFUNCTION()
	void HandleTaskFlowEvent(const FTaskFlowEvent& Event);
};
