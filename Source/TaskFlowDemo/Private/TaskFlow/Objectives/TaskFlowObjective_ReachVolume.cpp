#include "TaskFlow/Objectives/TaskFlowObjective_ReachVolume.h"

#include "Nodes/TaskFlowAction.h"
#include "TaskFlow/Actions/TaskFlowAction_ReachVolume.h"

#define LOCTEXT_NAMESPACE "TaskFlowObjective_ReachVolume"

UTaskFlowObjective_ReachVolume::UTaskFlowObjective_ReachVolume()
{
	ObjectiveDescription = LOCTEXT("DefaultDescription", "Reach Volume");
}

FText UTaskFlowObjective_ReachVolume::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Completes when a character enters the specified TaskFlowVolume.");
}

void UTaskFlowObjective_ReachVolume::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// [CN] 把节点上的过滤配置下发给 ReachVolume 动作，再走父类的动作执行流程。
	if (UTaskFlowAction_ReachVolume* Action = FindReachVolumeAction())
	{
		Action->VolumeID = VolumeID;
		Action->VolumeTag = VolumeTag;
	}

	Super::Execute_Implementation(Runner);
}

UTaskFlowAction_ReachVolume* UTaskFlowObjective_ReachVolume::FindReachVolumeAction() const
{
	for (const TObjectPtr<UTaskFlowAction>& ActionPtr : Actions)
	{
		if (UTaskFlowAction_ReachVolume* Action = Cast<UTaskFlowAction_ReachVolume>(ActionPtr.Get()))
		{
			return Action;
		}
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
