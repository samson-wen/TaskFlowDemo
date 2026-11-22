#include "TaskFlow/Actions/TaskFlowAction_AcceptTask.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowSystem.h"
#include "TaskFlowTypes.h"
#include "TaskFlow/GameTaskFlowTypes.h"

#include "Variant_Combat/Player/PlayerStateTaskComponent.h"

#define LOCTEXT_NAMESPACE "TaskFlowAction_ReachVolume"

// Sample action that logs a message. Default message is set in the constructor.
UTaskFlowAction_AcceptTask::UTaskFlowAction_AcceptTask()
{

}

FText UTaskFlowAction_AcceptTask::GetActionName() const
{
	return LOCTEXT("TaskFlowAction_AcceptTask", "Accept Task");
}

void UTaskFlowAction_AcceptTask::Execute_Implementation(UTaskFlowNode_Objective* OwnerNode)
{
	ACharacter* Character = Cast<ACharacter>(GetContext());
	APlayerState* PS = IsValid(Character) ? Character->GetPlayerState() : nullptr;
	UPlayerStateTaskComponent* PSTaskComponent = IsValid(PS) ? PS->GetComponentByClass< UPlayerStateTaskComponent>() : nullptr;
	if (PSTaskComponent != nullptr)
	{
		PSTaskComponent->AcceptTask(TaskId);
	}
	Super::Execute_Implementation(OwnerNode);
}

void UTaskFlowAction_AcceptTask::Finish()
{
	Super::Finish();
}

void UTaskFlowAction_AcceptTask::OnFlowEnded()
{
	Super::OnFlowEnded();
}

#undef LOCTEXT_NAMESPACE
