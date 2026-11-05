#pragma once

#include "CoreMinimal.h"
#include "TaskFlowEvent.h"
#include "UObject/Object.h"
#include "TaskFlowAction.generated.h"

class AActor;
class APlayerState;
class AGameStateBase;
class UTaskFlowNode_Objective;
class UTaskFlowSystem;

/**
 * [EN] Base class for task actions: an entry in the Actions array of UTaskFlowNode_Objective.
 * [CN] 任务动作基类：UTaskFlowNode_Objective 的 Actions 数组里的一项。
 * [EN] Developers inherit from this class (C++ or Blueprint) and implement Execute to define a custom action.
 * [CN] 开发者继承本类（C++ 或蓝图）并实现 Execute 即可自定义动作。
 * [EN] - Synchronous action: call Finish() directly inside Execute.
 * [CN] - 同步动作：Execute 里直接调用 Finish()；
 * [EN] - Asynchronous action: do not call Finish() inside Execute; call Finish() when the work is done (timer, callback, etc.).
 * [CN] - 异步动作：Execute 里不调用 Finish()，等事情做完（定时器、回调等）再调用 Finish()。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Task Action"))
class TASKFLOWGRAPH_API UTaskFlowAction : public UObject
{
	GENERATED_BODY()

public:
	/** [EN] Action display name; defaults to the class name (without the TaskFlowAction_ prefix). [CN] 动作显示名，默认取类名（去掉 TaskFlowAction_ 前缀） */
	virtual FText GetActionName() const;

	/** [EN] Execute the action; OwnerNode is the owning Objective node. [CN] 执行动作，OwnerNode 为所属目标节点 */
	UFUNCTION(BlueprintNativeEvent, Category = "TaskFlow")
	void Execute(UTaskFlowNode_Objective* OwnerNode);

	/** [EN] Notify the owning node that this action is finished, continuing the next action. [CN] 通知所属节点本动作已完成，继续执行下一个动作 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	virtual void Finish();

	/**
	 * [EN] Called when the whole flow ends (End / Stop). The base marks the action finished so late callbacks
	 * cannot call Finish() and advance the flow; async subclasses override to release external bindings.
	 * [CN] 整个流程结束（到达 End / 被停止）时调用；基类把动作标记为已完成，使迟到的回调无法再调用 Finish() 推进流程；
	 * 异步子类需重写以释放外部绑定（委托、定时器等）。
	 */
	virtual void OnFlowEnded();

	/** [EN] Whether finished. [CN] 是否已完成 */
	bool IsFinished() const { return bFinished; }

	/** [EN] Internal: called by the Objective node to trigger Execute. [CN] 内部：由目标节点调用，触发 Execute */
	void Start(UTaskFlowNode_Objective* InOwnerNode);

	/** [EN] Get the owning Objective node. [CN] 获取所属目标节点 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode_Objective* GetOwnerObjective() const { return OwningObjective.Get(); }

	/** [EN] Get the context (PlayerState or GameState) executing this action; may be null. [CN] 获取执行该动作的上下文（PlayerState 或 GameState），可能为空 */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AActor* GetContext() const;

	/** [EN] Get the PlayerState for a personal task (valid for personal tasks). [CN] 获取个人任务所属的 PlayerState（个人任务时有效） */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	APlayerState* GetPlayerStateOwner() const;

	/** [EN] Get the GameState for a global task (valid for global tasks). [CN] 获取全局任务所属的 GameState（全局任务时有效） */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AGameStateBase* GetGameStateOwner() const;

	/** [EN] Get the TaskFlowSystem driving the current flow (via the owning Objective's Runner, falling back to the world subsystem). [CN] 获取正在驱动当前流程的 TaskFlowSystem（经由所属目标节点的 Runner，回退到世界子系统） */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	UTaskFlowSystem* GetTaskFlowSystem() const;

	/** [EN] Set the current progress of the owning Objective node. [CN] 设置所属目标节点的当前进度 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Objective")
	void SetObjectiveProgress(int32 Value);

	/** [EN] Add to the current progress of the owning Objective node. [CN] 增加所属目标节点的当前进度 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Objective")
	void AddObjectiveProgress(int32 Amount);

	/** [EN] Called by the owning Objective when a task flow event is received; override in subclasses. [CN] 所属目标节点收到任务流事件时调用，子类重写以处理 */
	virtual void ReceiveTaskFlowEvent(const FTaskFlowEvent& Event);

	//~ Begin Blackboard read/write (forwarded to the owning Objective node)
	//~ 黑板读写（转发到所属目标节点）
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardInt(FName Key, int32 Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardInt(FName Key, int32& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardFloat(FName Key, float Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardFloat(FName Key, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardBool(FName Key, bool Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardBool(FName Key, bool& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardName(FName Key, FName Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardName(FName Key, FName& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardString(FName Key, const FString& Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardString(FName Key, FString& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardObject(FName Key, UObject* Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardObject(FName Key, UObject*& OutValue) const;
	//~ End Blackboard read/write

protected:
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowNode_Objective> OwningObjective;

	UPROPERTY(Transient)
	bool bFinished = false;
};
