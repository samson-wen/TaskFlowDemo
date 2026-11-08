#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowAction.h"
#include "TaskFlowAction_Log.generated.h"

/** Sample action: prints a log message then finishes immediately. */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Log"))
class TASKFLOWGRAPH_API UTaskFlowAction_Log : public UTaskFlowAction
{
	GENERATED_BODY()

public:
	UTaskFlowAction_Log();

	virtual FText GetActionName() const override;
	virtual void Execute_Implementation(UTaskFlowNode_Objective* OwnerNode) override;

	/** The content to print. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log")
	FString Message;
};
