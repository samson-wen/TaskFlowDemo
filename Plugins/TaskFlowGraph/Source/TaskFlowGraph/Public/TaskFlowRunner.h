#pragma once

#include "CoreMinimal.h"
#include "TaskFlowBlackboard.h"
#include "TimerManager.h"
#include "UObject/Object.h"
#include "TaskFlowRunner.generated.h"

class UTaskFlowAsset;
class UTaskFlowNode;
class UWorld;

/** 流程执行完毕广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskFlowFinished, UTaskFlowRunner*, Runner);

/**
 * 任务流程执行器。
 *
 * 用法：UTaskFlowRunner::StartTaskFlow(WorldContext, TaskFlowAsset)，
 * 执行器会从 Start 节点开始，沿着连线依次执行每个节点，直到没有后继节点（或到达 End 节点）。
 * 一个节点的输出引脚连了多个节点时，会形成多条分支，所有分支都执行完才算流程结束。
 */
UCLASS(BlueprintType)
class TASKFLOWGRAPH_API UTaskFlowRunner : public UObject
{
	GENERATED_BODY()

public:
	/** 开始执行一个任务流程，返回执行器实例 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Start Task Flow"))
	static UTaskFlowRunner* StartTaskFlow(const UObject* WorldContextObject, UTaskFlowAsset* InAsset);

	/** 使用已有实例开始执行流程，世界通过 Outer（例如 UTaskFlowSystem）解析 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void Start(UTaskFlowAsset* InAsset);

	/** 中断流程 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	bool IsRunning() const;

	/** 当前正在等待完成的节点（同步执行时通常为空） */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode* GetCurrentNode() const;

	/** 执行期间所在的 World，节点里可以用它拿 TimerManager 做异步逻辑 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UWorld* GetTaskFlowWorld() const;

	/** 节点完成时回调，用于推进流程 */
	void NotifyNodeFinished(UTaskFlowNode* Node);

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
	//~ End 黑板读写

	/** 正在执行的流程资产 */
	UPROPERTY(BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<UTaskFlowAsset> Asset;

	/** 流程执行完毕（所有分支结束或被中断）时广播 */
	UPROPERTY(BlueprintAssignable, Category = "TaskFlow")
	FOnTaskFlowFinished OnFinished;

	/** 节点之间步进的间隔（秒），0 表示同步一口气执行完；大于 0 时可看到节点逐个执行 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	float StepDelaySeconds = 0.f;

protected:
	/** 驱动流程往下走，处理同步节点时不会递归 */
	void Advance();

	/** 步进模式下，安排下一次推进 */
	void ScheduleAdvance();

	/** 清除步进定时器 */
	void ClearAdvanceTimer();

	void FinishFlow();

protected:
	/** 正在等待完成的节点 */
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowNode> CurrentNode;

	/** 待执行的节点（分支） */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTaskFlowNode>> PendingNodes;

	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> WorldPtr;

	/** 运行时黑板值（Key -> 值），由资产黑板初始化 */
	UPROPERTY(Transient)
	TMap<FName, FTaskFlowBlackboardEntry> BlackboardValues;

	UPROPERTY(Transient)
	bool bRunning = false;

	bool bAdvancing = false;

	/** 步进模式使用的定时器句柄 */
	FTimerHandle AdvanceTimerHandle;
};
