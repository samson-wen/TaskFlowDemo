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

	/** [EN] Broadcast an event globally to active receivers of all running task flows. [CN] 全局广播事件：发送给所有正在执行的任务流的活跃接收者 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void BroadcastEvent(FName EventName, UObject* Sender = nullptr, const FString& Payload = TEXT(""));

	/** [EN] Broadcast an event to a specific runner only. [CN] 定向广播事件：只发送给指定 Runner */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void BroadcastEventToRunner(UTaskFlowRunner* Runner, FName EventName, UObject* Sender = nullptr, const FString& Payload = TEXT(""));

	/** [EN] Broadcast to runners associated with the given context (PlayerState / GameState). [CN] 按上下文定向广播：只发送给指定 Context（PlayerState / GameState）关联的 Runner */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void BroadcastEventToContext(AActor* Context, FName EventName, UObject* Sender = nullptr, const FString& Payload = TEXT(""));

	/**
	 * [EN] Outbound event channel: fired for every event emitted by a running task flow (via UTaskFlowRunner::EmitEvent).
	 * [CN] 出站事件通道：流程内部通过 UTaskFlowRunner::EmitEvent 发出的每个事件都会触发此委托。
	 * [EN] External code (gameplay Blueprints / C++) binds here to observe the flow from the outside world.
	 * [CN] 外部代码（游戏逻辑蓝图 / C++）绑定到这里即可从外界监听流程。
	 */
	UPROPERTY(BlueprintAssignable, Category = "TaskFlow")
	FOnTaskFlowEvent OnTaskFlowEvent;

	/** [EN] Register an external object (must implement the TaskFlowEventReceiver interface) to receive outbound events. [CN] 注册一个外界对象（需实现 TaskFlowEventReceiver 接口）以接收出站事件 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void RegisterExternalReceiver(TScriptInterface<ITaskFlowEventReceiver> Receiver);

	/** [EN] Unregister an external receiver. [CN] 取消注册外界接收者 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void UnregisterExternalReceiver(TScriptInterface<ITaskFlowEventReceiver> Receiver);

	/** [EN] Broadcast an outbound event to the external world (global delegate + registered receivers). Called by UTaskFlowRunner::EmitEvent. [CN] 向外广播出站事件（全局委托 + 已注册的外部接收者）。由 UTaskFlowRunner::EmitEvent 调用 */
	void BroadcastEventToExternal(const FTaskFlowEvent& Event);

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

	/** [EN] External objects registered to receive outbound events (held weakly so destroyed objects don't leak). [CN] 注册以接收出站事件的外界对象（弱引用持有，销毁后自动失效不泄漏） */
	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> ExternalReceivers;
};
