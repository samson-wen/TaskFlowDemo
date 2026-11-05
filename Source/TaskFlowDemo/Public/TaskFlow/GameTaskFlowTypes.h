#pragma once

#include "CoreMinimal.h"
#include "TaskFlowEvent.h"
#include "GameTaskFlowTypes.generated.h"


namespace GameTaskFlowEvents
{
	
	const FName EVENT_TASKFLOW_VOLUME_ENTER(TEXT("EVENT_TASKFLOW_VOLUME_ENTER"));
	const FName EVENT_TASKFLOW_VOLUME_LEAVE(TEXT("EVENT_TASKFLOW_VOLUME_LEAVE"));

	const FName EVENT_ENEMY_DEATH(TEXT("EVENT_ENEMY_DEATH"));

}

/**
 * [EN] Payload for volume enter/leave events, stored in FTaskFlowEvent::EventData.
 * [CN] 体积进入 / 离开事件的负载，存放在 FTaskFlowEvent::EventData 中。
 *
 * [EN] Deliberately NOT derived from FTaskFlowEvent: it is a standalone payload, and the event metadata
 * (EventName / Sender / Payload) lives on the outer FTaskFlowEvent.
 * [CN] 刻意不继承 FTaskFlowEvent：它是独立负载，事件元信息（EventName / Sender / Payload）由外层 FTaskFlowEvent 承担。
 */
USTRUCT(BlueprintType)
struct FTaskFlowVolumeEvent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	int32 VolumeID = 0;

	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	FString VolumeTag = "";
};


/**
 * [EN] Payload for EVENT_ENEMY_DEATH, stored in FTaskFlowEvent::EventData.
 * [CN] 敌人死亡事件的负载，存放在 FTaskFlowEvent::EventData 中。
 */
USTRUCT(BlueprintType)
struct FTaskFlowKillTargetEvent
{
	GENERATED_BODY()

public:
	/** [EN] Task-flow identifier of the dead enemy; 0 means unassigned. [CN] 死亡敌人在任务流中的标识；0 表示未指定 */
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	int64 AIID = 0;
};

