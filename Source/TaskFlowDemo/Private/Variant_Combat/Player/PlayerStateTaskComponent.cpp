// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Combat/Player/PlayerStateTaskComponent.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "TaskFlow/GameTaskFlowTypes.h"
#include "TaskFlowAsset.h"
#include "TaskFlowSystem.h"
#include "TaskFlowTypes.h"
#include "Variant_Combat/Gameplay/GameEvents.h"


UPlayerStateTaskComponent::UPlayerStateTaskComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerStateTaskComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerStateTaskComponent::AcceptTask(int32 TaskId)
{
	if (TaskId <= 0 || CompletedTaskIds.Contains(TaskId) || AcceptedTaskIds.Contains(TaskId))
	{
		return;
	}

	UDataTable* TaskTable = TaskDataTable.LoadSynchronous();
	if (TaskTable == nullptr)
	{
		UE_LOG(LogTaskFlow, Warning, TEXT("AcceptTask: TaskDataTable is not configured or failed to load (TaskId=%d)."), TaskId);
		return;
	}

	// The table's RowName is normally the TaskID. Fall back to scanning the TaskID field so a table
	// configured with descriptive row names still works.
	FGameTask* TaskRow = TaskTable->FindRow<FGameTask>(FName(*FString::FromInt(TaskId)), TEXT("UPlayerStateTaskComponent::AcceptTask"));
	if (TaskRow == nullptr)
	{
		for (const FName& RowName : TaskTable->GetRowNames())
		{
			FGameTask* Candidate = TaskTable->FindRow<FGameTask>(RowName, TEXT("UPlayerStateTaskComponent::AcceptTask"));
			if (Candidate != nullptr && Candidate->TaskID == TaskId)
			{
				TaskRow = Candidate;
				break;
			}
		}
	}

	if (TaskRow == nullptr)
	{
		UE_LOG(LogTaskFlow, Warning, TEXT("AcceptTask: no FGameTask row for TaskId=%d."), TaskId);
		return;
	}

	UTaskFlowAsset* FlowAsset = TaskRow->TaskFlowAsset.LoadSynchronous();
	if (FlowAsset == nullptr)
	{
		UE_LOG(LogTaskFlow, Warning, TEXT("AcceptTask: TaskId=%d has no TaskFlowAsset."), TaskId);
		return;
	}

	UTaskFlowSystem* TaskFlowSystem = UTaskFlowSystem::Get(this);
	if (TaskFlowSystem == nullptr)
	{
		return;
	}

	// The owning PlayerState is the context, so the flow is treated as a personal task.
	APlayerState* OwnerPlayerState = Cast<APlayerState>(GetOwner());
	UTaskFlowRunner* Runner = TaskFlowSystem->RunTaskFlow(FlowAsset, OwnerPlayerState);
	if (Runner == nullptr)
	{
		UE_LOG(LogTaskFlow, Warning, TEXT("AcceptTask: RunTaskFlow failed for TaskId=%d."), TaskId);
		return;
	}

	AcceptedTaskIds.Add(TaskId);

	if (UGameEvents* GameEvents = UGameEvents::Get(this))
	{
		GameEvents->OnAcceptTaskEvt.Broadcast(TaskId, OwnerPlayerState);
	}

	UE_LOG(LogTaskFlow, Log, TEXT("AcceptTask: TaskId=%d started flow %s for %s."),
		TaskId, *FlowAsset->GetName(), *GetNameSafe(OwnerPlayerState));
}