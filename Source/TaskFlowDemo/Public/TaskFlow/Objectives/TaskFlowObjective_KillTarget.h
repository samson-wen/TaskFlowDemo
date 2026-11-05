#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowObjective_KillTarget.generated.h"

class UTaskFlowRunner;

/**
 * [EN] Specialized Objective node that completes when the required number of enemies die.
 * [CN] 专用目标节点：击杀指定数量的敌人后完成。
 *
 * [EN] Handles EVENT_ENEMY_DEATH directly via OnTaskFlowEventReceived; no Action needs to be added.
 * The base class owns the event binding/unbinding (cached System) and the flow-ended guard.
 * [CN] 通过 OnTaskFlowEventReceived 直接处理 EVENT_ENEMY_DEATH，无需在 Actions 里挂任何动作；
 * 事件委托的绑定/解绑（缓存 System）与流程结束拦截都由基类负责。
 *
 * [EN] Set MaxProgress (inherited >= 1) to require multiple kills. AIID = 0 matches any enemy.
 * [CN] 需要多次击杀时把继承来的 MaxProgress 设为 N；AIID = 0 表示匹配任意敌人。
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

public:
	/** [EN] Only count deaths of the enemy with this AIID; 0 means any enemy. [CN] 仅统计该 AIID 的敌人死亡；0 表示任意敌人 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kill Target")
	int64 AIID = 0;
};
