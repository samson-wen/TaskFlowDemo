#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "Subsystems/WorldSubsystem.h"
#include "TaskFlowEvent.h"
#include "TaskFlowSystem.generated.h"

class AActor;
class UTaskFlowAsset;
class UTaskFlowRunner;

/**
 * World subsystem that owns and drives task flows.
 * Its lifetime follows the UWorld. It is responsible for starting, holding and stopping running task flows.
 *
 * Usage: UWorld::GetSubsystem<UTaskFlowSystem>() or the "Get TaskFlowSystem" node in Blueprints,
 * then call Run Task Flow and pass a UTaskFlowAsset.
 */
UCLASS(BlueprintType)
class TASKFLOWGRAPH_API UTaskFlowSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	static UTaskFlowSystem* Get(const UObject* WorldContext);

	/** Start executing a task flow and return the runner instance. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowRunner* RunTaskFlow(UTaskFlowAsset* InAsset, AActor* InContext = nullptr);

	/**
	 * Whether any active Runner is currently executing this asset. Used by sub-flow nodes to reject
	 * loops (A->B->A) and concurrent runs of the same asset, whose runtime state lives on the shared asset.
	 * Because node runtime state lives on the shared asset, multiple Runners would overwrite each other.
	 */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	bool IsTaskFlowAssetRunning(const UTaskFlowAsset* InAsset) const;

	/** Stop a specific running task flow. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void StopTaskFlow(UTaskFlowRunner* InRunner);

	/** Stop all running task flows. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void StopAllTaskFlows();

	/** Find a node in the asset by name; returns nullptr if not found. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode* FindNodeByName(UTaskFlowAsset* InAsset, FName NodeName) const;

	/** Get the runtime state of a node: NotExecuted / Executing / Succeeded (returns NotExecuted if the node is not found). */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow", meta = (DisplayName = "Get Node State"))
	ETaskFlowNodeState GetNodeState(UTaskFlowAsset* InAsset, FName NodeName) const;

	/** Broadcast a task flow event: it is delivered to all listeners bound to the OnTaskFlowEvent delegate (Blueprint or C++). */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void BroadcastEvent(FName EventName, UObject* Sender = nullptr, const FString& Payload = TEXT(""));

	/** Broadcast a structured task flow event to all active runners. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void BroadcastTaskFlowEvent(const FTaskFlowEvent& Event);

	/**
	 * Outbound event channel: fired for every event emitted by a running task flow (via UTaskFlowRunner::EmitEvent).
	 * External code (gameplay Blueprints / C++) binds here to observe the flow from the outside world.
	 */
	UPROPERTY(BlueprintAssignable, Category = "TaskFlow")
	FOnTaskFlowEvent OnTaskFlowEvent;

	/** Step interval (seconds) between nodes. 0 means run everything synchronously; >0 lets you watch nodes execute one by one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	float StepDelaySeconds = 0.5f;

	//~ Begin USubsystem Interface
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

private:
	/** Callback when a flow finishes, used to remove it from the active list. */
	UFUNCTION()
	void HandleTaskFlowFinished(UTaskFlowRunner* Runner);

	/** Currently running task flows. */
	UPROPERTY()
	TArray<TObjectPtr<UTaskFlowRunner>> ActiveRunners;
};
