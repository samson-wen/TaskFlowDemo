#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/Object.h"
#include "TaskFlowEvent.generated.h"

/**
 * [EN] Task flow event: broadcast by UTaskFlowSystem; receivers handle it through the unified entry ReceiveTaskFlowEvent.
 * [CN] 任务流事件：由 UTaskFlowSystem 广播，接收者通过统一入口 ReceiveTaskFlowEvent 处理。
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
};

/** [EN] Dynamic multicast delegate for task flow events (Blueprint-bindable). External code binds to UTaskFlowSystem::OnTaskFlowEvent to observe outbound events. [CN] 任务流事件动态多播委托（可在蓝图中绑定）。外部代码绑定到 UTaskFlowSystem::OnTaskFlowEvent 即可监听出站事件 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskFlowEvent, const FTaskFlowEvent&, Event);

/** [EN] Unified interface for receiving task flow events. [CN] 任务流事件接收统一接口 */
UINTERFACE(Blueprintable)
class UTaskFlowEventReceiver : public UInterface
{
	GENERATED_BODY()
};

class TASKFLOWGRAPH_API ITaskFlowEventReceiver
{
	GENERATED_BODY()

public:
	/** [EN] Unified entry for receiving events; override in subclasses / Blueprints. [CN] 统一的事件接收处理入口，子类 / 蓝图重写 */
	UFUNCTION(BlueprintNativeEvent, Category = "TaskFlow")
	void ReceiveTaskFlowEvent(const FTaskFlowEvent& Event);
};
