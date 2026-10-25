#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TaskFlowSystem.generated.h"

class UTaskFlowAsset;
class UTaskFlowRunner;

/**
 * 任务流程世界子系统。
 * 生命周期跟随 UWorld，负责启动、持有和停止正在执行的任务流程。
 *
 * 使用：UWorld::GetSubsystem<UTaskFlowSystem>() 或蓝图里的 Get TaskFlowSystem 节点，
 * 然后调用 Run Task Flow 传入一个 UTaskFlowAsset。
 */
UCLASS(BlueprintType)
class TASKFLOWGRAPH_API UTaskFlowSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 开始执行一个任务流程，返回执行器实例 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowRunner* RunTaskFlow(UTaskFlowAsset* InAsset);

	/** 停止某个正在执行的任务流程 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void StopTaskFlow(UTaskFlowRunner* InRunner);

	/** 停止所有正在执行的任务流程 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void StopAllTaskFlows();

	/** 节点之间步进间隔（秒），0 表示同步一口气执行完；大于 0 时可看到节点逐个执行 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	float StepDelaySeconds = 0.5f;

	//~ Begin USubsystem Interface
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

private:
	/** 某个流程执行完毕时回调，用于把它从活动列表里移除 */
	UFUNCTION()
	void HandleTaskFlowFinished(UTaskFlowRunner* Runner);

	/** 当前正在执行的任务流程 */
	UPROPERTY()
	TArray<TObjectPtr<UTaskFlowRunner>> ActiveRunners;
};
