#include "TaskFlow/Objectives/TaskFlowObjective_KillTarget.h"

#include "TaskFlow/GameTaskFlowTypes.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowObjective_KillTarget"

UTaskFlowObjective_KillTarget::UTaskFlowObjective_KillTarget()
{
	ObjectiveDescription = LOCTEXT("DefaultDescription", "Kill Target");

	// This Objective completes on kill progress, so it requires at least 1; with MaxProgress = 0 the base class would complete immediately when there are no Actions.
	MaxProgress = 1;
	bListenToTaskFlowEvents = true;
}

FText UTaskFlowObjective_KillTarget::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Completes when the required number of enemies with the matching AIID die.");
}

void UTaskFlowObjective_KillTarget::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// Prevent the user from setting MaxProgress to 0 in the Details panel, which would complete the Objective without any kill.
	if (MaxProgress < 1)
	{
		MaxProgress = 1;
	}

	Super::Execute_Implementation(Runner);
}

bool UTaskFlowObjective_KillTarget::ShouldListenToTaskFlowEvents() const
{
	// Kill progress depends entirely on EVENT_ENEMY_DEATH, so it is unaffected by the base class default / an accidental toggle off.
	return true;
}

void UTaskFlowObjective_KillTarget::OnTaskFlowEventReceived(const FTaskFlowEvent& Event)
{
	const FString EnterDetail = FString::Printf(TEXT("EventName=%s"), *Event.EventName.ToString());
	LogTaskFlowLifecycle(
		TEXT("<None>"), TEXT("HookEntered"), GetName(),
		*GetClass()->GetName(), *EnterDetail);

	if (Event.EventName != GameTaskFlowEvents::EVENT_ENEMY_DEATH)
	{
		return;
	}

	// EventData is a type-erased payload; GetPtr validates the UScriptStruct type, so nothing is sliced.
	const FTaskFlowKillTargetEvent* KillEvt = Event.EventData.GetPtr<FTaskFlowKillTargetEvent>();
	if (KillEvt == nullptr)
	{
		LogTaskFlowLifecycle(
			TEXT("<None>"), TEXT("KillRejected"), GetName(),
			*GetClass()->GetName(), TEXT("EventData is not FTaskFlowKillTargetEvent"));
		return;
	}

	if (AIID != 0 && KillEvt->AIID != AIID)
	{
		const FString RejectDetail = FString::Printf(TEXT("ExpectedAIID=%lld ActualAIID=%lld"), AIID, KillEvt->AIID);
		LogTaskFlowLifecycle(
			TEXT("<None>"), TEXT("KillRejected"), GetName(),
			*GetClass()->GetName(), *RejectDetail);
		return;
	}

	// When progress reaches the target, the base TryComplete() unbinds the event and calls FinishExecution().
	AddProgress(1);

	const FString Detail = FString::Printf(TEXT("AIID=%lld Progress=%d/%d"),
		KillEvt->AIID, CurrentProgress, MaxProgress);
	LogTaskFlowLifecycle(
		TEXT("<None>"), TEXT("KillAccepted"), GetName(),
		*GetClass()->GetName(), *Detail);
}

#undef LOCTEXT_NAMESPACE
