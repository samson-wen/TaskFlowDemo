#include "TaskFlow/Actions/TaskFlowAction_ReachVolume.h"
#include "TaskFlowSystem.h"
#include "TaskFlow/GameTaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowAction_ReachVolume"

// [EN] Sample action that logs a message. Default message is set in the constructor.
// [CN] 示例动作：打印一条日志。构造函数设置了默认消息。
UTaskFlowAction_ReachVolume::UTaskFlowAction_ReachVolume()
{

}

FText UTaskFlowAction_ReachVolume::GetActionName() const
{
	return LOCTEXT("TaskFlowAction_ReachVolume", "Reach Volume");
}

void UTaskFlowAction_ReachVolume::Execute_Implementation(UTaskFlowNode_Objective* OwnerNode)
{
	// [CN] 异步动作：不在 Execute 中完成，改为自行订阅系统的 OnTaskFlowEvent，
	//      等待体积进入事件（不再依赖目标节点转发）。
	UTaskFlowSystem* System = GetTaskFlowSystem();
	if (System != nullptr)
	{
		// [CN] 先清理可能残留的绑定（同一动作实例会跨多次运行复用），再绑定。
		System->OnTaskFlowEvent.RemoveAll(this);
		System->OnTaskFlowEvent.AddDynamic(this, &UTaskFlowAction_ReachVolume::OnTaskFlowEventReceived);
	}
}

void UTaskFlowAction_ReachVolume::OnTaskFlowEventReceived(const FTaskFlowEvent& Event)
{
	// [CN] 已完成的动作不再处理（防御性，避免重复触发）。
	if (IsFinished())
	{
		return;
	}

	// [CN] 事件名不匹配直接忽略。
	if (Event.EventName != GameTaskFlowEvents::EVENT_TASKFLOW_VOLUME_ENTER)
	{
		return;
	}

	// [CN] 从类型擦除的负载中按类型安全取回体积数据；非体积事件 / 未填负载时返回 nullptr。
	//      （FInstancedStruct 自带 UScriptStruct，GetPtr 会做类型校验，不再需要向下转型）
	const FTaskFlowVolumeEvent* VolumeEvt = Event.EventData.GetPtr<FTaskFlowVolumeEvent>();
	if (VolumeEvt == nullptr)
	{
		return;
	}

	// [CN] 按配置的过滤条件比对；未配置的条件不参与过滤。
	if (VolumeID != 0 && VolumeEvt->VolumeID != VolumeID)
	{
		return;
	}
	if (!VolumeTag.IsNone() && FName(VolumeEvt->VolumeTag) != VolumeTag)
	{
		return;
	}

	AddObjectiveProgress(1);
	// [CN] 命中：完成本动作，目标节点将推进到下一个动作 / 完成。
	Finish();
}

void UTaskFlowAction_ReachVolume::Finish()
{
	Super::Finish();

	// [CN] 完成后解除对 OnTaskFlowEvent 的绑定（Broadcast 内部会拷贝调用列表，此处修改实时列表是安全的）。
	if (UTaskFlowSystem* System = GetTaskFlowSystem())
	{
		System->OnTaskFlowEvent.RemoveAll(this);
	}
}
#undef LOCTEXT_NAMESPACE
