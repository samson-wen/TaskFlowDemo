#pragma once

#include "CoreMinimal.h"
#include "TaskFlowBlackboard.h"
#include "TimerManager.h"
#include "UObject/Object.h"
#include "TaskFlowRunner.generated.h"

class AActor;
class AGameStateBase;
class APlayerState;
class UTaskFlowAsset;
class UTaskFlowNode;
class UTaskFlowSystem;
class UWorld;

struct FTaskFlowEvent;

/** [EN] Broadcasted when a task flow finishes. [CN] 流程执行完毕广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskFlowFinished, UTaskFlowRunner*, Runner);

/**
 * [EN] Task flow executor.
 * [CN] 任务流程执行器。
 *
 * [EN] Usage: UTaskFlowRunner::StartTaskFlow(WorldContext, TaskFlowAsset).
 * [CN] 用法：UTaskFlowRunner::StartTaskFlow(WorldContext, TaskFlowAsset)，
 * [EN] The executor starts from the Start node and follows the connections, executing each node in turn until there is no successor (or it reaches an End node).
 * [CN] 执行器会从 Start 节点开始，沿着连线依次执行每个节点，直到没有后继节点（或到达 End 节点）。
 * [EN] When one node's output pin connects to multiple nodes, it forms multiple branches; all branches must finish before the flow ends.
 * [CN] 一个节点的输出引脚连了多个节点时，会形成多条分支，所有分支都执行完才算流程结束。
 */
UCLASS(BlueprintType)
class TASKFLOWGRAPH_API UTaskFlowRunner : public UObject
{
	GENERATED_BODY()

public:
	/** [EN] Start executing a task flow and return the runner instance. [CN] 开始执行一个任务流程，返回执行器实例；InContext 传入 PlayerState 表示个人任务，传入 GameState 表示全局任务 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Start Task Flow"))
	static UTaskFlowRunner* StartTaskFlow(const UObject* WorldContextObject, UTaskFlowAsset* InAsset, AActor* InContext = nullptr);

	/** [EN] Start executing with an existing instance; the World is resolved via the Outer (e.g. UTaskFlowSystem). [CN] 使用已有实例开始执行流程，世界通过 Outer（例如 UTaskFlowSystem）解析 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void Start(UTaskFlowAsset* InAsset, AActor* InContext = nullptr);

	/** [EN] Abort the flow. [CN] 中断流程 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	bool IsRunning() const;

	/** [EN] The node currently waiting to finish (usually null during synchronous execution). [CN] 当前正在等待完成的节点（同步执行时通常为空） */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode* GetCurrentNode() const;

	/** [EN] The World during execution; nodes can use it to get the TimerManager for async logic. [CN] 执行期间所在的 World，节点里可以用它拿 TimerManager 做异步逻辑 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UWorld* GetTaskFlowWorld() const;

	/** [EN] Called when a node finishes, to advance the flow. [CN] 节点完成时回调，用于推进流程 */
	void NotifyNodeFinished(UTaskFlowNode* Node);

	/** [EN] Emit a task flow event outward to the external world (external code / registered receivers). [CN] 从流程内部向外界发出事件（外界代码 / 已注册的外部接收者） */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Event", meta = (DisplayName = "Emit Task Flow Event"))
	void EmitEvent(const FTaskFlowEvent& Event);

	/** [EN] Convenience overload to emit an outbound event by name. [CN] 按名称发出出站事件的便捷重载 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Event", meta = (DisplayName = "Emit Task Flow Event By Name"))
	void EmitEventByName(FName EventName, UObject* Sender = nullptr, const FString& Payload = TEXT(""));

	/** [EN] Resolve the owning TaskFlowSystem via the World. [CN] 通过 World 解析所属的 TaskFlowSystem */
	UTaskFlowSystem* GetTaskFlowSystem() const;

	//~ Begin Blackboard read/write (internal; nodes/actions forward here via their own UFUNCTIONs)
	//~ 黑板读写（内部使用，节点/动作通过各自的 UFUNCTION 转发到这里）
	bool SetBlackboardInt(FName Key, int32 Value);
	bool GetBlackboardInt(FName Key, int32& OutValue) const;

	bool SetBlackboardFloat(FName Key, float Value);
	bool GetBlackboardFloat(FName Key, float& OutValue) const;

	bool SetBlackboardBool(FName Key, bool Value);
	bool GetBlackboardBool(FName Key, bool& OutValue) const;

	bool SetBlackboardName(FName Key, FName Value);
	bool GetBlackboardName(FName Key, FName& OutValue) const;

	bool SetBlackboardString(FName Key, const FString& Value);
	bool GetBlackboardString(FName Key, FString& OutValue) const;

	bool SetBlackboardObject(FName Key, UObject* Value);
	bool GetBlackboardObject(FName Key, UObject*& OutValue) const;
	//~ End Blackboard read/write

	/** [EN] The context passed in (PlayerState, GameState, etc.). [CN] 传入的上下文（PlayerState 或 GameState 等） */
	UPROPERTY(BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<AActor> Context;

	/** [EN] The PlayerState that owns a personal task (passing a PlayerState means a personal task). [CN] 个人任务所属的 PlayerState（传入 PlayerState 表示个人任务） */
	UPROPERTY(BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<APlayerState> PlayerStateOwner;

	/** [EN] The GameState that owns a global task (passing a GameState means a global task). [CN] 全局任务所属的 GameState（传入 GameState 表示全局任务） */
	UPROPERTY(BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<AGameStateBase> GameStateOwner;

	/** [EN] Get the passed-in context. [CN] 获取传入的上下文 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	AActor* GetContext() const { return Context.Get(); }

	/** [EN] Get the PlayerState that owns the personal task. [CN] 获取个人任务所属的 PlayerState */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	APlayerState* GetPlayerStateOwner() const { return PlayerStateOwner.Get(); }

	/** [EN] Get the GameState that owns the global task. [CN] 获取全局任务所属的 GameState */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	AGameStateBase* GetGameStateOwner() const { return GameStateOwner.Get(); }

	/** [EN] Whether this is a personal task (a PlayerState was passed in). [CN] 是否为个人任务（传入了 PlayerState） */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	bool IsPersonalTask() const { return PlayerStateOwner.Get() != nullptr; }

	/** [EN] Whether this is a global task (a GameState was passed in). [CN] 是否为全局任务（传入了 GameState） */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	bool IsGlobalTask() const { return GameStateOwner.Get() != nullptr; }

	/** [EN] The task flow asset being executed. [CN] 正在执行的流程资产 */
	UPROPERTY(BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<UTaskFlowAsset> Asset;

	/** [EN] Broadcasted when the flow finishes (all branches end or it is aborted). [CN] 流程执行完毕（所有分支结束或被中断）时广播 */
	UPROPERTY(BlueprintAssignable, Category = "TaskFlow")
	FOnTaskFlowFinished OnFinished;

	/** [EN] Step interval (seconds) between nodes. 0 means run everything synchronously; >0 lets you watch nodes execute one by one. [CN] 节点之间步进的间隔（秒），0 表示同步一口气执行完；大于 0 时可看到节点逐个执行 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	float StepDelaySeconds = 0.f;

protected:
	/** [EN] Drive the flow forward; does not recurse for synchronous nodes. [CN] 驱动流程往下走，处理同步节点时不会递归 */
	void Advance();

	/** [EN] Schedule the next advance in step mode. [CN] 步进模式下，安排下一次推进 */
	void ScheduleAdvance();

	/** [EN] Clear the step timer. [CN] 清除步进定时器 */
	void ClearAdvanceTimer();

	void FinishFlow();

	/** [EN] Validate that the context type matches the asset's task type (editor-only popup). [CN] 校验上下文类型与资产任务类型是否一致（仅编辑器下弹窗提示） */
	void ValidateContextType(UTaskFlowAsset* InAsset, AActor* InContext);

protected:
	/** [EN] The node currently waiting to finish. [CN] 正在等待完成的节点 */
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowNode> CurrentNode;

	/** [EN] Nodes (branches) pending execution. [CN] 待执行的节点（分支） */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTaskFlowNode>> PendingNodes;

	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> WorldPtr;

	/** [EN] Runtime blackboard values (Key -> value), initialized from the asset blackboard. [CN] 运行时黑板值（Key -> 值），由资产黑板初始化 */
	UPROPERTY(Transient)
	TMap<FName, FTaskFlowBlackboardEntry> BlackboardValues;

	UPROPERTY(Transient)
	bool bRunning = false;

	bool bAdvancing = false;

	/** [EN] Timer handle used in step mode. [CN] 步进模式使用的定时器句柄 */
	FTimerHandle AdvanceTimerHandle;
};
