#include "TaskFlow/Objectives/TaskFlowObjective_AcceptTask.h"

#include "GameFramework/GameState.h"
#include "TaskFlow/GameTaskFlowTypes.h"
#include "TaskFlowTypes.h"
#include "Variant_Combat/Player/PlayerStateTaskComponent.h"

#define LOCTEXT_NAMESPACE "TaskFlowObjective_AcceptTask"

UTaskFlowObjective_AcceptTask::UTaskFlowObjective_AcceptTask()
{
	ObjectiveDescription = LOCTEXT("DefaultDescription", "Accept Task");
}

FText UTaskFlowObjective_AcceptTask::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Waits until the player accepts the configured task, then completes.");
}

void UTaskFlowObjective_AcceptTask::Execute_Implementation(UTaskFlowRunner* Runner)
{
	if (TaskID <= 0)
	{
		return;
	}
	AActor* ContextActor = GetContext();
	if (IsValid(ContextActor) == false)
	{
		return;
	}
	APlayerState* PS = Cast<APlayerState>(ContextActor);
	if (PS != nullptr)
	{
		AcceptTask(PS, TaskID);
	}
	else 
	{
		AGameStateBase* GS = Cast<AGameStateBase>(ContextActor);
		if (GS != nullptr)
		{
			for (APlayerState* EachPS : GS->PlayerArray)
			{
				AcceptTask(EachPS, TaskID);
			}
		}
	}
	Super::Execute_Implementation(Runner);
}


void UTaskFlowObjective_AcceptTask::AcceptTask(APlayerState* PS, int32 InTaskID)
{
	UPlayerStateTaskComponent* PSTaskComponent = IsValid(PS) ? PS->GetComponentByClass< UPlayerStateTaskComponent>() : nullptr;
	if (PSTaskComponent != nullptr)
	{
		PSTaskComponent->AcceptTask(InTaskID);
	}
}

#undef LOCTEXT_NAMESPACE
