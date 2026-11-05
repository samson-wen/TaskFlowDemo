#include "TaskFlow/Actions/TaskFlowAction_ReachVolume.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowSystem.h"
#include "TaskFlow/GameTaskFlowTypes.h"
#include "TaskFlowTypes.h"

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
		// [CN] 先清理可能残留的绑定（同一动作实例会跨多次运行复用），再绑定，并缓存所用 System。
		UnbindFromTaskFlowEvent();
		System->OnTaskFlowEvent.AddDynamic(this, &UTaskFlowAction_ReachVolume::OnTaskFlowEventReceived);
		BoundSystem = System;

		LogTaskFlowLifecycle(
			GetNameSafe(GetOwnerObjective()), TEXT("EventBound"), GetName(),
			*GetClass()->GetName(), TEXT("OnTaskFlowEvent"));
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
	// [CN] 命中：完成本动作，目标节点将推进到下一个动作 / 完成。
	Finish();
}

void UTaskFlowAction_ReachVolume::Finish()
{
	Super::Finish();

	UnbindFromTaskFlowEvent();
}

void UTaskFlowAction_ReachVolume::OnFlowEnded()
{
	// [CN] 流程已结束：先解绑全局事件监听，再交给基类把本动作标记为已完成。
	UnbindFromTaskFlowEvent();
	Super::OnFlowEnded();
}

void UTaskFlowAction_ReachVolume::UnbindFromTaskFlowEvent()
{
	// [CN] 只使用绑定时缓存的 System：此时 ActiveRunner 往往已被清空，GetTaskFlowSystem() 会返回 nullptr，
	//      若依赖它解绑就会整块跳过，导致委托残留、后续事件仍能触发本动作。
	//      （Broadcast 内部拷贝调用列表，此处修改实时列表是安全的。）
	UTaskFlowSystem* System = BoundSystem.Get();
	if (System != nullptr)
	{
		System->OnTaskFlowEvent.RemoveAll(this);

		LogTaskFlowLifecycle(
			GetNameSafe(GetOwnerObjective()), TEXT("EventUnbound"), GetName(),
			*GetClass()->GetName(), TEXT("OnTaskFlowEvent"));
	}

	BoundSystem.Reset();
}
#undef LOCTEXT_NAMESPACE
