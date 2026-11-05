#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/Object.h"
#include "StructUtils/InstancedStruct.h"
#include "TaskFlowEvent.generated.h"

/**
 * [EN] Task flow event: broadcast by UTaskFlowSystem via the OnTaskFlowEvent delegate; nodes / actions subscribe to that delegate to handle events.
 * [CN] 任务流事件：由 UTaskFlowSystem 通过 OnTaskFlowEvent 委托广播；节点 / 动作订阅该委托即可处理事件。
 *
 * [EN] Event-specific data is carried by EventData (FInstancedStruct). USTRUCT has no polymorphism and a dynamic
 * delegate marshals parameters by the declared base type, so inheriting from FTaskFlowEvent and down-casting would slice
 * the derived fields. FInstancedStruct keeps its own UScriptStruct and deep-copies, so the full payload survives.
 * [CN] 事件专属数据放在 EventData（FInstancedStruct）里。USTRUCT 不支持多态，动态委托按声明的基类类型编组参数，
 * 继承 FTaskFlowEvent 再向下转型会切掉派生字段；FInstancedStruct 自带 UScriptStruct 并深拷贝，可完整保留负载。
 */
USTRUCT(BlueprintType)
struct FTaskFlowEvent
{
	GENERATED_BODY()

	/** [EN] Event identifier used by receivers to filter (an empty EventName means no reception). [CN] 事件标识，接收者用它过滤（EventName 为空则不接收） */
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	FName EventName = NAME_None;

	/** [EN] Event sender. [CN] 事件发送者 */
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	TObjectPtr<UObject> Sender = nullptr;

	/** [EN] Attached payload. [CN] 附加负载 */
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	FString Payload;

	/** [EN] Type-erased structured payload. Use GetPtr<T>() to read event-specific data (e.g. FTaskFlowVolumeEvent). [CN] 类型擦除的结构化负载，用 GetPtr<T>() 读取事件专属数据（如 FTaskFlowVolumeEvent） */
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	FInstancedStruct EventData;
};

/** [EN] Dynamic multicast delegate for task flow events (Blueprint-bindable). External code binds to UTaskFlowSystem::OnTaskFlowEvent to observe outbound events. [CN] 任务流事件动态多播委托（可在蓝图中绑定）。外部代码绑定到 UTaskFlowSystem::OnTaskFlowEvent 即可监听出站事件 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskFlowEvent, const FTaskFlowEvent&, Event);


