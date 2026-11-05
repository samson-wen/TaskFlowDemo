// Copyright Epic Games, Inc. All Rights Reserved.

#include "TaskFlow/GameTaskFlowStatics.h"

#include "Variant_Combat/AI/CombatEnemy.h"
#include "TaskFlowEvent.h"
#include "TaskFlowSystem.h"
#include "TaskFlow/GameTaskFlowTypes.h"
#include "TaskFlowTypes.h"

void UGameTaskFlowStatics::BroadcastKillTargetTaskFlow(ACharacter* InKillerCharacter, ACharacter* InDeathCharacter)
{
	// [CN] 死亡角色必须有效；击杀者可能为空（环境伤害 / 伤害来源已销毁），此时仍要广播，Sender 留空。
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

	// [CN] 击杀专属数据放进类型擦除的 EventData，经动态委托派发时会被深拷贝，不会被切片。
	FTaskFlowKillTargetEvent& KillTargetEvt = TaskFlowEvt.EventData.InitializeAs<FTaskFlowKillTargetEvent>();

	// [CN] AIID 取自死亡敌人本体；非 ACombatEnemy 的死亡不携带标识（0 = 任意）。
	const ACombatEnemy* DeadEnemy = Cast<ACombatEnemy>(InDeathCharacter);
	KillTargetEvt.AIID = DeadEnemy != nullptr ? DeadEnemy->GetAIID() : 0;

	TaskFlowSystem->BroadcastTaskFlowEvent(TaskFlowEvt);

	const FString Detail = FString::Printf(TEXT("AIID=%lld Killer=%s"), KillTargetEvt.AIID, *GetNameSafe(InKillerCharacter));
	LogTaskFlowLifecycle(
		TEXT("<None>"), TEXT("KillBroadcast"), GetNameSafe(InDeathCharacter),
		*InDeathCharacter->GetClass()->GetName(), *Detail);
}
