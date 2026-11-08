#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "EdGraph/EdGraphNode.h"
#include "TaskFlowNode.generated.h"

class AActor;
class APlayerState;
class AGameStateBase;
class UEdGraphPin;
class UTaskFlowGraphSchema;
class UTaskFlowNode;
class UTaskFlowRunner;

/** Runtime state of a task flow node. */
UENUM(BlueprintType)
enum class ETaskFlowNodeState : uint8
{
	/** Not executed yet. */
	NotExecuted UMETA(DisplayName = "Not Executed"),
	/** Currently executing. */
	Executing   UMETA(DisplayName = "Executing"),
	/** Executed successfully. */
	Succeeded   UMETA(DisplayName = "Succeeded")
};

/** Broadcast when a node's state changes (used by the editor to refresh node colors at runtime). */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTaskFlowNodeStateChanged, UTaskFlowNode*);

/**
 * Base class for task flow nodes.
 *
 * Developers inherit from this class (C++ or Blueprint) and implement Execute to define their own task node.
 * - Synchronous node: do the work directly inside Execute; the base implementation automatically calls FinishExecution to advance the flow.
 * - Asynchronous node: do not call the base implementation inside Execute; call FinishExecution manually when the work is done (timer, callback, etc.).
 *
 * A node has one input pin and one output pin by default, and can be freely wired in the graph.
 * The Start node has no input pin; the End node has no output pin.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, meta = (DisplayName = "Task Node"))
class TASKFLOWGRAPH_API UTaskFlowNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UTaskFlowNode();

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* InSchema) const override;
	virtual bool CanUserDeleteNode() const override;
	virtual bool CanDuplicateNode() const override;
	//~ End UEdGraphNode Interface

	/** Broadcast when a node's state changes. */
	static FOnTaskFlowNodeStateChanged& OnNodeStateChanged();

	/** Get the current node state. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	ETaskFlowNodeState GetNodeState() const;

	/** Set the node state (called by the Runner). */
	void SetNodeState(ETaskFlowNodeState InState);

	/** Return the node title color based on state: gray=not executed, yellow=executing, green=succeeded. */
	virtual FLinearColor GetStateColor() const;

	/** Category shown in the right-click menu; reads the UCLASS metadata TaskFlowCategory by default. */
	virtual FText GetNodeCategory() const;

	/** Node name, used to query runtime state by name; returns the object name by default (subclasses can override, e.g. Objective returns NodeName). */
	virtual FName GetNodeName() const;

	/** Whether only one instance of this node is allowed in the same graph (true for the Start node). */
	virtual bool IsUniqueNode() const;

	/** Input pin; null for the Start node. */
	UEdGraphPin* GetInputPin() const;

	/** Output pin; null for the End node. */
	UEdGraphPin* GetOutputPin() const;

	/** The first successor node. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode* GetNextNode() const;

	/** All successor nodes; multiple branches exist when the output pin connects to several (subclasses can override to choose which output to follow). */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	virtual void GetNextNodes(TArray<UTaskFlowNode*>& OutNextNodes) const;

	/**
	 * Entry point of node logic, called by UTaskFlowRunner.
	 * Synchronous nodes call Super::Execute(Runner) to finish automatically; asynchronous nodes call FinishExecution at the appropriate time.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "TaskFlow")
	void Execute(UTaskFlowRunner* Runner);

	/** Notify the flow to continue; asynchronous nodes must call this exactly once when their logic ends. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void FinishExecution();

	/**
	 * Called by the Runner when the whole flow ends (reaches End / finishes / is stopped).
	 * Subclasses must abort pending async work and release external bindings; the base detaches the Runner.
	 */
	virtual void OnFlowEnded();

	/**
	 * Whether the Runner should mark this node Succeeded when it finishes.
	 * Merge nodes (e.g. AND) return false until all their inputs have arrived, so they stay yellow.
	 */
	virtual bool ShouldMarkSucceededOnFinish() const;

	/** Internal entry called by the Runner to record the Runner and trigger Execute. */
	void BeginExecution(UTaskFlowRunner* Runner);

	/** Get the Runner currently executing this node (valid only during Execute). */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowRunner* GetRunner() const;

	/** Get the context (PlayerState, GameState, etc.) executing this node; may be null. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	AActor* GetContext() const;

	/** Get the PlayerState for a personal task (valid for personal tasks). */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	APlayerState* GetPlayerStateOwner() const;

	/** Get the GameState for a global task (valid for global tasks). */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AGameStateBase* GetGameStateOwner() const;

	//~ Begin Blackboard read/write (forwarded to the current Runner)
	//~ Blackboard read/write (forwarded to the current Runner)
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
	/** Collect successor nodes from the given output pin. */
	void CollectNodesFromPin(const UEdGraphPin* Pin, TArray<UTaskFlowNode*>& OutNodes) const;

	/** The Runner currently executing this node. */
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowRunner> ActiveRunner;

	/** Runtime state of the node. */
	UPROPERTY(Transient)
	ETaskFlowNodeState NodeState = ETaskFlowNodeState::NotExecuted;
};
