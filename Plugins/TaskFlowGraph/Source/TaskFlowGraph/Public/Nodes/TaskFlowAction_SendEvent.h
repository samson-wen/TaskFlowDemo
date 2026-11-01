#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowAction.h"
#include "TaskFlowAction_SendEvent.generated.h"

/**
 * [EN] Action that emits an outbound task flow event to the outside world.
 * [CN] 向外界发出一条出站任务流事件的动作。
 * [EN] Place it inside an Objective node's Actions array; when executed it calls the Runner's EmitEvent,
 * [EN] which forwards the event to UTaskFlowSystem::OnTaskFlowEvent and any registered external receivers.
 * [CN] 把它放进某个目标节点的 Actions 数组；执行时会调用 Runner 的 EmitEvent，
 * [CN] 由 UTaskFlowSystem::OnTaskFlowEvent 与已注册的外界接收者收到。
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Send Event"))
class TASKFLOWGRAPH_API UTaskFlowAction_SendEvent : public UTaskFlowAction
{
	GENERATED_BODY()

public:
	UTaskFlowAction_SendEvent();

	virtual FText GetActionName() const override;
	virtual void Execute_Implementation(UTaskFlowNode_Objective* OwnerNode) override;

	/** [EN] Event name sent to the outside world. [CN] 发往外界的事件名 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SendEvent")
	FName EventName = NAME_None;

	/** [EN] Optional payload string attached to the event. [CN] 附加在事件上的可选负载字符串 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SendEvent")
	FString Payload;
};
