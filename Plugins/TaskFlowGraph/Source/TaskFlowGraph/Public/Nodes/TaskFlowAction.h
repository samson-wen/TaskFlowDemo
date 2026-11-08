#pragma once

#include "CoreMinimal.h"
#include "TaskFlowEvent.h"
#include "UObject/Object.h"
#include "TaskFlowAction.generated.h"

class AActor;
class APlayerState;
class AGameStateBase;
class UTaskFlowNode_Objective;
class UTaskFlowSystem;

/**
 * Base class for task actions: an entry in the Actions array of UTaskFlowNode_Objective.
 * Developers inherit from this class (C++ or Blueprint) and implement Execute to define a custom action.
 * - Synchronous action: call Finish() directly inside Execute.
 * - Asynchronous action: do not call Finish() inside Execute; call Finish() when the work is done (timer, callback, etc.).
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Task Action"))
class TASKFLOWGRAPH_API UTaskFlowAction : public UObject
{
	GENERATED_BODY()

public:
	/** Action display name; defaults to the class name (without the TaskFlowAction_ prefix). */
	virtual FText GetActionName() const;

	/** Execute the action; OwnerNode is the owning Objective node. */
	UFUNCTION(BlueprintNativeEvent, Category = "TaskFlow")
	void Execute(UTaskFlowNode_Objective* OwnerNode);

	/** Notify the owning node that this action is finished, continuing the next action. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	virtual void Finish();

	/**
	 * Called when the whole flow ends (End / Stop). The base marks the action finished so late callbacks
	 * cannot call Finish() and advance the flow; async subclasses override to release external bindings.
	 * Async subclasses should override this to release external bindings (delegates, timers, etc.).
	 */
	virtual void OnFlowEnded();

	/** Whether finished. */
	bool IsFinished() const { return bFinished; }

	/** Internal: called by the Objective node to trigger Execute. */
	void Start(UTaskFlowNode_Objective* InOwnerNode);

	/** Get the owning Objective node. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode_Objective* GetOwnerObjective() const { return OwningObjective.Get(); }

	/** Get the context (PlayerState or GameState) executing this action; may be null. */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AActor* GetContext() const;

	/** Get the PlayerState for a personal task (valid for personal tasks). */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	APlayerState* GetPlayerStateOwner() const;

	/** Get the GameState for a global task (valid for global tasks). */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AGameStateBase* GetGameStateOwner() const;

	/** Get the TaskFlowSystem driving the current flow (via the owning Objective's Runner, falling back to the world subsystem). */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	UTaskFlowSystem* GetTaskFlowSystem() const;

	/** Set the current progress of the owning Objective node. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Objective")
	void SetObjectiveProgress(int32 Value);

	/** Add to the current progress of the owning Objective node. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Objective")
	void AddObjectiveProgress(int32 Amount);

	/** Called by the owning Objective when a task flow event is received; override in subclasses. */
	virtual void ReceiveTaskFlowEvent(const FTaskFlowEvent& Event);

	//~ Begin Blackboard read/write (forwarded to the owning Objective node)
	//~ Blackboard read/write (forwarded to the owning Objective node)
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardInt(FName Key, int32 Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardInt(FName Key, int32& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardFloat(FName Key, float Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardFloat(FName Key, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardBool(FName Key, bool Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardBool(FName Key, bool& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardName(FName Key, FName Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardName(FName Key, FName& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardString(FName Key, const FString& Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardString(FName Key, FString& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Blackboard")
	bool SetBlackboardObject(FName Key, UObject* Value);
	UFUNCTION(BlueprintPure, Category = "TaskFlow|Blackboard")
	bool GetBlackboardObject(FName Key, UObject*& OutValue) const;
	//~ End Blackboard read/write

protected:
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowNode_Objective> OwningObjective;

	UPROPERTY(Transient)
	bool bFinished = false;
};
