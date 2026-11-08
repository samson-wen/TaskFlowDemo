#include "TaskFlow/Actions/TaskFlowAction_ReachVolume.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowSystem.h"
#include "TaskFlow/GameTaskFlowTypes.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowAction_ReachVolume"

// Sample action that logs a message. Default message is set in the constructor.
UTaskFlowAction_ReachVolume::UTaskFlowAction_ReachVolume()
{

}

FText UTaskFlowAction_ReachVolume::GetActionName() const
{
	return LOCTEXT("TaskFlowAction_ReachVolume", "Reach Volume");
}

void UTaskFlowAction_ReachVolume::Execute_Implementation(UTaskFlowNode_Objective* OwnerNode)
{
	// Asynchronous action: it does not finish inside Execute; instead it subscribes to the system's OnTaskFlowEvent
	//      and waits for the volume-enter event (it no longer relies on Objective node forwarding).
	UTaskFlowSystem* System = GetTaskFlowSystem();
	if (System != nullptr)
	{
		// Clear any leftover binding first (the same action instance is reused across runs), then bind and cache the System.
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
	// Ignore events once the action is finished (defensive, avoids duplicate triggers).
	if (IsFinished())
	{
		return;
	}

	// Ignore events whose name does not match.
	if (Event.EventName != GameTaskFlowEvents::EVENT_TASKFLOW_VOLUME_ENTER)
	{
		return;
	}

	// Retrieve volume data from the type-erased payload with a type check; returns nullptr for non-volume events / empty payloads.
	//      (FInstancedStruct carries its own UScriptStruct; GetPtr validates the type, so no down-cast is needed)
	const FTaskFlowVolumeEvent* VolumeEvt = Event.EventData.GetPtr<FTaskFlowVolumeEvent>();
	if (VolumeEvt == nullptr)
	{
		return;
	}

	// Compare against the configured filters; filters that are not configured are skipped.
	if (VolumeID != 0 && VolumeEvt->VolumeID != VolumeID)
	{
		return;
	}
	if (!VolumeTag.IsNone() && FName(VolumeEvt->VolumeTag) != VolumeTag)
	{
		return;
	}
	// Match: finish this action; the Objective node will advance to the next action / complete.
	Finish();
}

void UTaskFlowAction_ReachVolume::Finish()
{
	Super::Finish();

	UnbindFromTaskFlowEvent();
}

void UTaskFlowAction_ReachVolume::OnFlowEnded()
{
	// The flow has ended: unbind the global event listener first, then let the base class mark this action finished.
	UnbindFromTaskFlowEvent();
	Super::OnFlowEnded();
}

void UTaskFlowAction_ReachVolume::UnbindFromTaskFlowEvent()
{
	// Use only the System cached at bind time: ActiveRunner is often cleared by now and GetTaskFlowSystem() would return nullptr,
	//      which would skip unbinding entirely, leaving the delegate behind so later events still trigger this action.
	//      (Broadcast copies its invocation list internally, so modifying the live list here is safe.)
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
