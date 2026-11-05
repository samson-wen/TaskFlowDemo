#include "TaskFlow/Objectives/TaskFlowObjective_KillTarget.h"

#include "TaskFlow/GameTaskFlowTypes.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowObjective_KillTarget"

UTaskFlowObjective_KillTarget::UTaskFlowObjective_KillTarget()
{
	ObjectiveDescription = LOCTEXT("DefaultDescription", "Kill Target");

	// [CN] 该目标靠击杀进度完成，必须至少要求 1 次；MaxProgress 为 0 时基类会在无 Actions 的情况下立即判定完成。
	MaxProgress = 1;
}

FText UTaskFlowObjective_KillTarget::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Completes when the required number of enemies with the matching AIID die.");
}

void UTaskFlowObjective_KillTarget::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// [CN] 防止用户在 Details 面板把 MaxProgress 改成 0，导致目标无需击杀就立即完成。
	if (MaxProgress < 1)
	{
		MaxProgress = 1;
	}

	Super::Execute_Implementation(Runner);
}

void UTaskFlowObjective_KillTarget::OnTaskFlowEventReceived(const FTaskFlowEvent& Event)
{
	if (Event.EventName != GameTaskFlowEvents::EVENT_ENEMY_DEATH)
	{
		return;
	}

	// [CN] EventData 是类型擦除负载，GetPtr 会做 UScriptStruct 类型校验，不会切片。
	const FTaskFlowKillTargetEvent* KillEvt = Event.EventData.GetPtr<FTaskFlowKillTargetEvent>();
	if (KillEvt == nullptr)
	{
		return;
	}

	if (AIID != 0 && KillEvt->AIID != AIID)
	{
		return;
	}

	// [CN] 进度达标时基类 TryComplete() 会自动解绑事件并 FinishExecution()。
	AddProgress(1);

	const FString Detail = FString::Printf(TEXT("AIID=%lld"), KillEvt->AIID);
	LogTaskFlowLifecycle(
		TEXT("<None>"), TEXT("KillAccepted"), GetName(),
		*GetClass()->GetName(), *Detail);
}

#undef LOCTEXT_NAMESPACE
