#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/Object.h"
#include "StructUtils/InstancedStruct.h"
#include "TaskFlowEvent.generated.h"

/**
 * Task flow event: broadcast by UTaskFlowSystem via the OnTaskFlowEvent delegate; nodes / actions subscribe to that delegate to handle events.
 *
 * Event-specific data is carried by EventData (FInstancedStruct). USTRUCT has no polymorphism and a dynamic
 * delegate marshals parameters by the declared base type, so inheriting from FTaskFlowEvent and down-casting would slice
 * the derived fields. FInstancedStruct keeps its own UScriptStruct and deep-copies, so the full payload survives.
 * Deriving from FTaskFlowEvent and down-casting slices the derived fields; FInstancedStruct carries its own UScriptStruct and deep-copies, so the full payload survives.
 */
USTRUCT(BlueprintType)
struct FTaskFlowEvent
{
	GENERATED_BODY()

	/** Event identifier used by receivers to filter (an empty EventName means no reception). */
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	FName EventName = NAME_None;

	/** Event sender. */
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	TObjectPtr<UObject> Sender = nullptr;

	/** Attached payload. */
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	FString Payload;

	/** Type-erased structured payload. Use GetPtr<T>() to read event-specific data (e.g. FTaskFlowVolumeEvent). */
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	FInstancedStruct EventData;
};

/** Dynamic multicast delegate for task flow events (Blueprint-bindable). External code binds to UTaskFlowSystem::OnTaskFlowEvent to observe outbound events. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskFlowEvent, const FTaskFlowEvent&, Event);


