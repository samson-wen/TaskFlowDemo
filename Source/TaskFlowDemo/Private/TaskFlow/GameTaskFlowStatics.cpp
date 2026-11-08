// Copyright Epic Games, Inc. All Rights Reserved.

#include "TaskFlow/GameTaskFlowStatics.h"

#include "Variant_Combat/AI/CombatEnemy.h"
#include "TaskFlowEvent.h"
#include "TaskFlowSystem.h"
#include "TaskFlow/GameTaskFlowTypes.h"
#include "TaskFlowTypes.h"

void UGameTaskFlowStatics::BroadcastKillTargetTaskFlow(ACharacter* InKillerCharacter, ACharacter* InDeathCharacter)
{
	// The dead character must be valid; the killer may be null (environmental damage / destroyed instigator), in which case we still broadcast with an empty Sender.
	if (IsValid(InDeathCharacter) == false)
	{
		return;
	}

	UWorld* World = InDeathCharacter->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	UTaskFlowSystem* TaskFlowSystem = World->GetSubsystem<UTaskFlowSystem>();
	if (TaskFlowSystem == nullptr)
	{
		return;
	}

	FTaskFlowEvent TaskFlowEvt;
	TaskFlowEvt.EventName = GameTaskFlowEvents::EVENT_ENEMY_DEATH;
	TaskFlowEvt.Sender = InKillerCharacter;

	// Put kill-specific data into the type-erased EventData; the dynamic delegate deep-copies it, so it is not sliced.
	FTaskFlowKillTargetEvent& KillTargetEvt = TaskFlowEvt.EventData.InitializeAs<FTaskFlowKillTargetEvent>();

	// AIID comes from the dead enemy itself; non-ACombatEnemy deaths carry no identifier (0 = any).
	const ACombatEnemy* DeadEnemy = Cast<ACombatEnemy>(InDeathCharacter);
	KillTargetEvt.AIID = DeadEnemy != nullptr ? DeadEnemy->GetAIID() : 0;

	TaskFlowSystem->BroadcastTaskFlowEvent(TaskFlowEvt);

	const FString Detail = FString::Printf(TEXT("AIID=%lld Killer=%s"), KillTargetEvt.AIID, *GetNameSafe(InKillerCharacter));
	LogTaskFlowLifecycle(
		TEXT("<None>"), TEXT("KillBroadcast"), GetNameSafe(InDeathCharacter),
		*InDeathCharacter->GetClass()->GetName(), *Detail);
}
