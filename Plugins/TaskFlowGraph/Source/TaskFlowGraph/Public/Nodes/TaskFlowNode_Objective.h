#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowEvent.h"
#include "TaskFlowSystem.h"
#include "TaskFlowNode_Objective.generated.h"

class UTaskFlowAction;
class UTaskFlowRunner;

/**
 * [EN] Objective node: represents a goal / step in the task flow.
 * [CN] 目标节点：表示任务流程中的一个目标 / 步骤。
 * [EN] It inherits from UTaskFlowNode and can be created and wired directly in the TaskFlowGraph asset editor;
 * [EN] when triggered, it executes the actions in the Actions list in order, and advances to the next node once all are done.
 * [CN] 它继承自 UTaskFlowNode，可以直接在 TaskFlowGraph 资产编辑器里创建并连线；
 * [CN] 节点被触发后，会按顺序依次执行 Actions 列表里的动作，全部完成后推进到下一个节点。
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

	/** [EN] Flow ended: unbind from OnTaskFlowEvent, abort all pending Actions and ignore late callbacks. [CN] 流程结束：解除 OnTaskFlowEvent 绑定、中止所有未完成的动作，并忽略迟到的回调 */
	virtual void OnFlowEnded() override;

	/** [EN] Objective description; shown in the node tooltip and the Details panel. [CN] 目标内容描述，会显示在节点的悬浮提示和 Details 面板里 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (MultiLine = true))
	FText ObjectiveDescription;

	/** [EN] Node name, must be unique within the same asset (empty means unnamed, not validated). [CN] 节点名称，同一资产内不可重复（留空表示未命名，不校验） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName NodeName;

	//~ Begin UObject Interface
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

	/** [EN] List of actions to execute, in order. [CN] 该目标要执行的动作列表，按顺序依次执行 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", Instanced)
	TArray<TObjectPtr<UTaskFlowAction>> Actions;

	/** [EN] Maximum progress; 0 means no progress needed; >0 requires current progress to reach this value to complete. [CN] 最大进度，0 表示不需要进度；大于 0 时需当前进度达到该值才算完成 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (ClampMin = "0"))
	int32 MaxProgress = 0;

	/** [EN] Current progress (runtime); the Objective completes once it reaches MaxProgress. [CN] 当前进度（运行时），达到 MaxProgress 后目标才算完成 */
	UPROPERTY(BlueprintReadOnly, Category = "Objective", Transient)
	int32 CurrentProgress = 0;

	/** [EN] Add to the current progress. [CN] 增加当前进度 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void AddProgress(int32 Amount);

	/** [EN] Set the current progress. [CN] 设置当前进度 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetProgress(int32 Value);

	/** [EN] Internal: callback when an action finishes, to continue the next action. [CN] 内部：某个动作完成时回调，继续执行下一个动作 */
	void OnActionFinished(UTaskFlowAction* Action);

	/** [EN] Get the TaskFlowSystem driving the current flow (via the owning Objective's Runner, falling back to the world subsystem). [CN] 获取正在驱动当前流程的 TaskFlowSystem（经由所属目标节点的 Runner，回退到世界子系统） */
	UFUNCTION(BlueprintPure, Category = "Objective")
	UTaskFlowSystem* GetTaskFlowSystem() const;

protected:
	/**
	 * [EN] Hook invoked for every event received while this Objective is executing, BEFORE forwarding to Actions.
	 * Specialized Objective subclasses override this to handle events directly; the default does nothing.
	 * [CN] 本目标执行期间收到事件时调用（在转发给 Actions 之前）；专用目标子类重写以直接处理事件，默认不处理。
	 * [EN] The base already guards against flow-ended / non-executing states and handles binding/unbinding.
	 * [CN] 基类已负责流程结束 / 非执行状态的拦截，以及事件委托的绑定与解绑，子类无需重复处理。
	 */
	virtual void OnTaskFlowEventReceived(const FTaskFlowEvent& Event);

private:
	/** [EN] Determine whether the current NodeName duplicates another Objective node in the graph. [CN] 判断当前 NodeName 是否与图中其他目标节点重复 */
	bool IsNodeNameDuplicate() const;

	/** [EN] Continue executing the next action. [CN] 继续执行下一个动作 */
	void ExecuteNextAction();

	/** [EN] When all actions are done and progress is met, call FinishExecution to complete the node. [CN] 动作全部完成且进度达标时，调用 FinishExecution 完成节点 */
	void TryComplete();

	/** [EN] Unbind from OnTaskFlowEvent using the cached System. [CN] 用缓存的 System 解除 OnTaskFlowEvent 绑定 */
	void UnbindFromTaskFlowEvent();

	/** [EN] The exact System whose OnTaskFlowEvent this node bound to; cached at bind time. [CN] 绑定时记录所用 OnTaskFlowEvent 所属的 System */
	UPROPERTY(Transient)
	TWeakObjectPtr<UTaskFlowSystem> BoundSystem;

	/** [EN] Index of the action currently executing (or waiting to finish). [CN] 当前正在执行（或等待完成）的动作索引 */
	int32 CurrentActionIndex = 0;

	/** [EN] Prevent recursion caused by Finish() inside synchronous actions. [CN] 防止同步动作里 Finish 造成的递归 */
	bool bAdvancingActions = false;

	/** [EN] Whether all actions have been executed. [CN] 动作是否已全部执行完 */
	bool bActionsCompleted = false;

	/** [EN] Set once the flow ends; blocks late event / action callbacks from advancing this Objective. [CN] 流程结束后置位，阻止迟到的事件 / 动作回调继续推进本目标 */
	bool bFlowEnded = false;

	/** [EN] NodeName before editing, used to roll back on duplication. [CN] 编辑前的 NodeName，重复时用于回退 */
	FName OldNodeName;

	/** [EN] Internal handler bound to UTaskFlowSystem::OnTaskFlowEvent; forwards events to unfinished Actions. [CN] 绑定到 OnTaskFlowEvent 的内部处理：把事件转发给尚未完成的动作 */
	UFUNCTION()
	void HandleTaskFlowEvent(const FTaskFlowEvent& Event);
};
