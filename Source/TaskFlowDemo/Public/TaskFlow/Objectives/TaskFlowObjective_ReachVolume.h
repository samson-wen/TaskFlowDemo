#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowObjective_ReachVolume.generated.h"

class UTaskFlowAction_ReachVolume;
class UTaskFlowRunner;

/**
 * [EN] Specialized Objective node that completes when a character enters the specified TaskFlowVolume.
 * [CN] 专用目标节点：角色进入指定 TaskFlowVolume 时完成。
 *
 * [EN] Exposes VolumeID / VolumeTag on the node itself and forwards them to the first
 * UTaskFlowAction_ReachVolume in the Actions list when execution starts.
 * [CN] 节点上直接暴露 VolumeID / VolumeTag，开始执行时同步给 Actions 列表中的第一个 UTaskFlowAction_ReachVolume。
 *
 * [EN] Add a ReachVolume action to the Actions list to make this node wait for the volume-enter event.
 * [CN] 需在 Actions 列表里添加一个 ReachVolume 动作，本节点才会等待体积进入事件。
 */
UCLASS(Blueprintable, meta = (DisplayName = "Reach Volume Objective", TaskFlowCategory = "Task"))
class TASKFLOWDEMO_API UTaskFlowObjective_ReachVolume : public UTaskFlowNode_Objective
{
	GENERATED_BODY()

public:
	UTaskFlowObjective_ReachVolume();

	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;

	/** [EN] Only volumes with this id match; 0 means any. [CN] 仅匹配该 ID 的体积；0 表示任意 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReachVolume")
	int32 VolumeID = 0;

	/** [EN] Only volumes with this tag match; NAME_None means any. [CN] 仅匹配该标签的体积；留空表示任意 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReachVolume")
	FName VolumeTag = NAME_None;

private:
	/** [EN] Find the configured ReachVolume action, if any. [CN] 查找已配置的 ReachVolume 动作 */
	UTaskFlowAction_ReachVolume* FindReachVolumeAction() const;
};
