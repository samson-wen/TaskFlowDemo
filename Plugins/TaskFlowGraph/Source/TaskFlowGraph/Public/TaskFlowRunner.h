#pragma once

#include "CoreMinimal.h"
#include "TaskFlowBlackboard.h"
#include "TimerManager.h"
#include "UObject/Object.h"
#include "TaskFlowRunner.generated.h"

class AActor;
class AGameStateBase;
class APlayerState;
class UTaskFlowAsset;
class UTaskFlowNode;
class UTaskFlowSystem;
class UWorld;

struct FTaskFlowEvent;

/** Broadcasted when a task flow finishes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskFlowFinished, UTaskFlowRunner*, Runner);

/**
 * Task flow executor.
 *
 * Usage: UTaskFlowRunner::StartTaskFlow(WorldContext, TaskFlowAsset).
 * The executor starts from the Start node and follows the connections until there is no successor (or it reaches an End node).
 * When one node's output pin connects to multiple nodes, all of them start together as one parallel batch;
 * each branch then advances independently. The flow ends after every branch has finished, OR immediately when
 * any End node is reached (all other active / pending nodes are stopped in that case).
 * The flow ends when all branches finish, but executing any End node immediately terminates the whole flow and stops all other active / pending nodes.
 */
UCLASS(BlueprintType)
class TASKFLOWGRAPH_API UTaskFlowRunner : public UObject
{
	GENERATED_BODY()

public:
	/** Start executing a task flow and return the runner instance. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Start Task Flow"))
	static UTaskFlowRunner* StartTaskFlow(const UObject* WorldContextObject, UTaskFlowAsset* InAsset, AActor* InContext = nullptr);

	/** Start executing with an existing instance; the World is resolved via the Outer (e.g. UTaskFlowSystem). */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void Start(UTaskFlowAsset* InAsset, AActor* InContext = nullptr);

	/** Abort the flow. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	bool IsRunning() const;

	/** One of the nodes currently executing in parallel; returns nullptr when no branch is active. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode* GetCurrentNode() const;

	/** All nodes currently executing in parallel (one per active branch). */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void GetActiveNodes(TArray<UTaskFlowNode*>& OutNodes) const;

	/** The World during execution; nodes can use it to get the TimerManager for async logic. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UWorld* GetTaskFlowWorld() const;

	/** Called when a node finishes, to advance the flow. */
	void NotifyNodeFinished(UTaskFlowNode* Node);

	/** Emit a task flow event outward to the external world (external code / registered receivers). */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Event", meta = (DisplayName = "Emit Task Flow Event"))
	void EmitEvent(const FTaskFlowEvent& Event);

	/** Convenience overload to emit an outbound event by name. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Event", meta = (DisplayName = "Emit Task Flow Event By Name"))
	void EmitEventByName(FName EventName, UObject* Sender = nullptr, const FString& Payload = TEXT(""));

	/** Resolve the owning TaskFlowSystem via the World. */
	UTaskFlowSystem* GetTaskFlowSystem() const;

	//~ Begin Blackboard read/write (internal; nodes/actions forward here via their own UFUNCTIONs)
	//~ Blackboard read/write (internal; nodes / actions forward here through their own UFUNCTIONs)
	bool SetBlackboardInt(FName Key, int32 Value);
	bool GetBlackboardInt(FName Key, int32& OutValue) const;

	bool SetBlackboardFloat(FName Key, float Value);
	bool GetBlackboardFloat(FName Key, float& OutValue) const;

	bool SetBlackboardBool(FName Key, bool Value);
	bool GetBlackboardBool(FName Key, bool& OutValue) const;

	bool SetBlackboardName(FName Key, FName Value);
	bool GetBlackboardName(FName Key, FName& OutValue) const;

	bool SetBlackboardString(FName Key, const FString& Value);
	bool GetBlackboardString(FName Key, FString& OutValue) const;

	bool SetBlackboardObject(FName Key, UObject* Value);
	bool GetBlackboardObject(FName Key, UObject*& OutValue) const;
	//~ End Blackboard read/write

	/** The context passed in (PlayerState, GameState, etc.). */
	UPROPERTY(BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<AActor> Context;

	/** The PlayerState that owns a personal task (passing a PlayerState means a personal task). */
	UPROPERTY(BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<APlayerState> PlayerStateOwner;

	/** The GameState that owns a global task (passing a GameState means a global task). */
	UPROPERTY(BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<AGameStateBase> GameStateOwner;

	/** Get the passed-in context. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	AActor* GetContext() const { return Context.Get(); }

	/** Get the PlayerState that owns the personal task. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	APlayerState* GetPlayerStateOwner() const { return PlayerStateOwner.Get(); }

	/** Get the GameState that owns the global task. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	AGameStateBase* GetGameStateOwner() const { return GameStateOwner.Get(); }

	/** Whether this is a personal task (a PlayerState was passed in). */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	bool IsPersonalTask() const { return PlayerStateOwner.Get() != nullptr; }

	/** Whether this is a global task (a GameState was passed in). */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	bool IsGlobalTask() const { return GameStateOwner.Get() != nullptr; }

	/** The task flow asset being executed. */
	UPROPERTY(BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<UTaskFlowAsset> Asset;

	/** Broadcasted when the flow finishes (all branches end or it is aborted). */
	UPROPERTY(BlueprintAssignable, Category = "TaskFlow")
	FOnTaskFlowFinished OnFinished;

	/** Step interval (seconds) between nodes. 0 means run everything synchronously; >0 lets you watch nodes execute one by one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	float StepDelaySeconds = 0.f;

protected:
	/** Drive the flow forward; does not recurse for synchronous nodes. */
	void Advance();

	/** Schedule the next advance in step mode. */
	void ScheduleAdvance();

	/** Clear the step timer. */
	void ClearAdvanceTimer();

	void FinishFlow();

	/** Notify every node in the asset that the flow has ended, so Objective/Actions abort pending async work. */
	void NotifyNodesFlowEnded();

	/** Validate that the context type matches the asset's task type (editor-only popup). */
	void ValidateContextType(UTaskFlowAsset* InAsset, AActor* InContext);

protected:
	/** All nodes currently executing in parallel; each one is waiting for its own FinishExecution. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTaskFlowNode>> ActiveNodes;

	/** Nodes waiting to be started together as the next parallel batch. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTaskFlowNode>> PendingNodes;

	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> WorldPtr;

	/** Runtime blackboard values (Key -> value), initialized from the asset blackboard. */
	UPROPERTY(Transient)
	TMap<FName, FTaskFlowBlackboardEntry> BlackboardValues;

	UPROPERTY(Transient)
	bool bRunning = false;

	bool bAdvancing = false;

	/** Timer handle used in step mode. */
	FTimerHandle AdvanceTimerHandle;
};
