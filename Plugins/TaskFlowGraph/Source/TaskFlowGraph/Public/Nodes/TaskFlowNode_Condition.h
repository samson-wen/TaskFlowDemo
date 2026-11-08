#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TaskFlowNode_Condition.generated.h"

class AActor;
class APlayerState;
class AGameStateBase;
class UTaskFlowNode_IF;

/**
 * Base class for conditions: an entry in the Conditions list of UTaskFlowNode_IF.
 * Developers inherit from this class (C++ or Blueprint) and implement Evaluate to define a custom condition;
 * returning true means the condition is satisfied.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Condition"))
class TASKFLOWGRAPH_API UTaskFlowNode_Condition : public UObject
{
	GENERATED_BODY()

public:
	/** Condition display name; defaults to the class name (without the TaskFlowNode_Condition_ prefix). */
	virtual FText GetConditionName() const;

	/** Evaluate the condition; OwnerNode is the owning IF node. */
	UFUNCTION(BlueprintNativeEvent, Category = "TaskFlow")
	bool Evaluate(UTaskFlowNode_IF* OwnerNode);

	/** Whether it has been evaluated. */
	bool IsEvaluated() const { return bEvaluated; }

	/** The most recent evaluation result. */
	bool GetResult() const { return bResult; }

	/** Internal: called by the IF node to trigger Evaluate. */
	void Start(UTaskFlowNode_IF* InOwnerNode);

	/** Get the context (PlayerState or GameState) evaluating this condition; may be null. */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AActor* GetContext() const;

	/** Get the PlayerState for a personal task (valid for personal tasks). */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	APlayerState* GetPlayerStateOwner() const;

	/** Get the GameState for a global task (valid for global tasks). */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AGameStateBase* GetGameStateOwner() const;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowNode_IF> OwnerIFNode;

	UPROPERTY(Transient)
	bool bEvaluated = false;

	UPROPERTY(Transient)
	bool bResult = false;
};
