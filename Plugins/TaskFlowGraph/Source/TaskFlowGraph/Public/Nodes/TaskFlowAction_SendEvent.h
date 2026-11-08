#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowAction.h"
#include "TaskFlowAction_SendEvent.generated.h"

/**
 * Action that emits an outbound task flow event to the outside world.
 * Place it inside an Objective node's Actions array; when executed it calls the Runner's EmitEvent,
 * which forwards the event to UTaskFlowSystem::OnTaskFlowEvent and any registered external receivers.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Send Event"))
class TASKFLOWGRAPH_API UTaskFlowAction_SendEvent : public UTaskFlowAction
{
	GENERATED_BODY()

public:
	UTaskFlowAction_SendEvent();

	virtual FText GetActionName() const override;
	virtual void Execute_Implementation(UTaskFlowNode_Objective* OwnerNode) override;

	/** Event name sent to the outside world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SendEvent")
	FName EventName = NAME_None;

	/** Optional payload string attached to the event. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SendEvent")
	FString Payload;
};
