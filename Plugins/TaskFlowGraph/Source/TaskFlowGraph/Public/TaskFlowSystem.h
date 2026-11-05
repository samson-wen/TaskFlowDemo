#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "Subsystems/WorldSubsystem.h"
#include "TaskFlowEvent.h"
#include "TaskFlowSystem.generated.h"

class AActor;
class UTaskFlowAsset;
class UTaskFlowRunner;

/**
 * [EN] World subsystem that owns and drives task flows.
 * [CN] 任务流程世界子系统。
 * [EN] Its lifetime follows the UWorld. It is responsible for starting, holding and stopping running task flows.
 * [CN] 生命周期跟随 UWorld，负责启动、持有和停止正在执行的任务流程。
 *
 * [EN] Usage: UWorld::GetSubsystem<UTaskFlowSystem>() or the "Get TaskFlowSystem" node in Blueprints,
 * [EN] then call Run Task Flow and pass a UTaskFlowAsset.
 * [CN] 使用：UWorld::GetSubsystem<UTaskFlowSystem>() 或蓝图里的 Get TaskFlowSystem 节点，
 * [CN] 然后调用 Run Task Flow 传入一个 UTaskFlowAsset。
 */
UCLASS(BlueprintType)
class TASKFLOWGRAPH_API UTaskFlowSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	static UTaskFlowSystem* Get(const UObject* WorldContext);

	/** [EN] Start executing a task flow and return the runner instance. [CN] 开始执行一个任务流程，返回执行器实例；InContext 传入 PlayerState 表示个人任务，传入 GameState 表示全局任务 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowRunner* RunTaskFlow(UTaskFlowAsset* InAsset, AActor* InContext = nullptr);

	/** [EN] Stop a specific running task flow. [CN] 停止某个正在执行的任务流程 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void StopTaskFlow(UTaskFlowRunner* InRunner);

	/** [EN] Stop all running task flows. [CN] 停止所有正在执行的任务流程 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void StopAllTaskFlows();

	/** [EN] Find a node in the asset by name; returns nullptr if not found. [CN] 按节点名称查找资产中的节点，未找到返回 nullptr */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode* FindNodeByName(UTaskFlowAsset* InAsset, FName NodeName) const;

	/** [EN] Get the runtime state of a node: NotExecuted / Executing / Succeeded (returns NotExecuted if the node is not found). [CN] 获取某个节点的运行状态：未执行 / 执行中 / 已完成（未找到节点返回 NotExecuted） */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow", meta = (DisplayName = "Get Node State"))
	ETaskFlowNodeState GetNodeState(UTaskFlowAsset* InAsset, FName NodeName) const;

	/** [EN] Broadcast a task flow event: it is delivered to all listeners bound to the OnTaskFlowEvent delegate (Blueprint or C++). [CN] 广播任务流事件：派发给所有绑定到 OnTaskFlowEvent 委托的监听者（蓝图或 C++）。 [EN] The system no longer traverses its internal Runners / Objectives / Actions; any node or action that wants to handle events binds to OnTaskFlowEvent itself. [CN] 系统不再遍历内部的 Runner / Objective / Action；任何想要处理事件的节点或动作，自行绑定 OnTaskFlowEvent 即可。 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void BroadcastEvent(FName EventName, UObject* Sender = nullptr, const FString& Payload = TEXT(""));

	/** [EN] Broadcast a structured task flow event to all active runners. [CN] 以结构体形式广播任务流事件，发送给所有正在执行的 Runner */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void BroadcastTaskFlowEvent(const FTaskFlowEvent& Event);

	/**
	 * [EN] Outbound event channel: fired for every event emitted by a running task flow (via UTaskFlowRunner::EmitEvent).
	 * [CN] 出站事件通道：流程内部通过 UTaskFlowRunner::EmitEvent 发出的每个事件都会触发此委托。
	 * [EN] External code (gameplay Blueprints / C++) binds here to observe the flow from the outside world.
	 * [CN] 外部代码（游戏逻辑蓝图 / C++）绑定到这里即可从外界监听流程。
	 */
	UPROPERTY(BlueprintAssignable, Category = "TaskFlow")
	FOnTaskFlowEvent OnTaskFlowEvent;

	/** [EN] Step interval (seconds) between nodes. 0 means run everything synchronously; >0 lets you watch nodes execute one by one. [CN] 节点之间步进间隔（秒），0 表示同步一口气执行完；大于 0 时可看到节点逐个执行 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	float StepDelaySeconds = 0.5f;

	//~ Begin USubsystem Interface
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

private:
	/** [EN] Callback when a flow finishes, used to remove it from the active list. [CN] 某个流程执行完毕时回调，用于把它从活动列表里移除 */
	UFUNCTION()
	void HandleTaskFlowFinished(UTaskFlowRunner* Runner);

	/** [EN] Currently running task flows. [CN] 当前正在执行的任务流程 */
	UPROPERTY()
	TArray<TObjectPtr<UTaskFlowRunner>> ActiveRunners;
};
