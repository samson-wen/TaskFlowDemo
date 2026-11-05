#pragma once

#include "CoreMinimal.h"
#include "TaskFlowEvent.h"
#include "GameTaskFlowTypes.generated.h"


namespace GameTaskFlowEvents
{
	const FName EVENT_TASKFLOW_VOLUME_ENTER(TEXT("EVENT_TASKFLOW_VOLUME_ENTER"));
	const FName EVENT_TASKFLOW_VOLUME_LEAVE(TEXT("EVENT_TASKFLOW_VOLUME_LEAVE"));

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
