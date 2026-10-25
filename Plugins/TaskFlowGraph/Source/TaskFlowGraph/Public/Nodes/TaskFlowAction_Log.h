#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowAction.h"
#include "TaskFlowAction_Log.generated.h"

/** 示例动作：打印一条日志后立即完成 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "打印日志"))
class TASKFLOWGRAPH_API UTaskFlowAction_Log : public UTaskFlowAction
{
	GENERATED_BODY()

public:
	UTaskFlowAction_Log();

	virtual FText GetActionName() const override;
	virtual void Execute_Implementation(UTaskFlowNode_Objective* OwnerNode) override;

	/** 要打印的内容 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log")
	FString Message;
};
