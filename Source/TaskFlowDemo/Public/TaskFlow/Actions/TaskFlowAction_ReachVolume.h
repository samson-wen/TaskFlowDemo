#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowAction.h"
#include "TaskFlowAction_ReachVolume.generated.h"

/** [EN] Action that completes when the player reaches (enters) a TaskFlowVolume. [CN] 当玩家抵达（进入）指定 TaskFlowVolume 时完成的动作 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "ReachVolume"))
class TASKFLOWDEMO_API UTaskFlowAction_ReachVolume : public UTaskFlowAction
{
	GENERATED_BODY()

public:
	UTaskFlowAction_ReachVolume();

	virtual FText GetActionName() const override;
	virtual void Execute_Implementation(UTaskFlowNode_Objective* OwnerNode) override;

	/** [EN] Called by the system's OnTaskFlowEvent while this action is awaiting a volume-enter event. [CN] 本动作等待体积进入事件期间，由系统的 OnTaskFlowEvent 触发 */
	UFUNCTION()
	void OnTaskFlowEventReceived(const FTaskFlowEvent& Event);

	/** [EN] Override Finish to unbind from OnTaskFlowEvent. [CN] 重写 Finish 以解除对 OnTaskFlowEvent 的绑定 */
	virtual void Finish() override;

	/** [EN] Event name that means "reached the volume"; must match what the volume broadcasts. [CN] 表示“抵达体积”的事件名，需与体积广播的事件名一致 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReachVolume")
	int32 VolumeID = 0;

	/** [EN] Only finish when the entered volume's tag matches this value; empty means any volume. [CN] 仅当进入体积的 Tag 与此一致时才完成；留空表示任意体积 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReachVolume")
	FName VolumeTag = NAME_None;
};
