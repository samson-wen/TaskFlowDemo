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

/** [EN] Runtime state of a task flow node. [CN] 任务流节点运行时状态 */
UENUM(BlueprintType)
enum class ETaskFlowNodeState : uint8
{
	/** [EN] Not executed yet. [CN] 未执行 */
	NotExecuted UMETA(DisplayName = "未执行"),
	/** [EN] Currently executing. [CN] 正在执行 */
	Executing   UMETA(DisplayName = "执行中"),
	/** [EN] Executed successfully. [CN] 执行成功 */
	Succeeded   UMETA(DisplayName = "已成功")
};

/** [EN] Broadcast when a node's state changes (used by the editor to refresh node colors at runtime). [CN] 节点状态变化广播（编辑器用它在运行时刷新节点颜色） */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTaskFlowNodeStateChanged, UTaskFlowNode*);

/**
 * [EN] Base class for task flow nodes.
 * [CN] 任务流节点基类。
 *
 * [EN] Developers inherit from this class (C++ or Blueprint) and implement Execute to define their own task node.
 * [CN] 开发者继承本类（C++ 或蓝图）并实现 Execute 即可定义自己的任务节点。
 * [EN] - Synchronous node: do the work directly inside Execute; the base implementation automatically calls FinishExecution to advance the flow.
 * [CN] - 同步节点：在 Execute 里直接做完事情，基类实现会自动调用 FinishExecution 推进流程；
 * [EN] - Asynchronous node: do not call the base implementation inside Execute; call FinishExecution manually when the work is done (timer, callback, etc.).
 * [CN] - 异步节点：Execute 里不调用基类实现，等事情做完（定时器、回调等）再手动调用 FinishExecution。
 *
 * [EN] A node has one input pin and one output pin by default, and can be freely wired in the graph.
 * [CN] 节点默认拥有一个输入引脚和一个输出引脚，可在图中自由连线。
 * [EN] The Start node has no input pin; the End node has no output pin.
 * [CN] Start 节点没有输入引脚，End 节点没有输出引脚。
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

	/** [EN] Broadcast when a node's state changes. [CN] 节点状态变化广播 */
	static FOnTaskFlowNodeStateChanged& OnNodeStateChanged();

	/** [EN] Get the current node state. [CN] 获取当前节点状态 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	ETaskFlowNodeState GetNodeState() const;

	/** [EN] Set the node state (called by the Runner). [CN] 设置节点状态（由 Runner 调用） */
	void SetNodeState(ETaskFlowNodeState InState);

	/** [EN] Return the node title color based on state: gray=not executed, yellow=executing, green=succeeded. [CN] 根据状态返回节点标题颜色：灰色=未执行，黄色=执行中，绿色=成功 */
	virtual FLinearColor GetStateColor() const;

	/** [EN] Category shown in the right-click menu; reads the UCLASS metadata TaskFlowCategory by default. [CN] 右键菜单中的分类，默认读取 UCLASS 元数据 TaskFlowCategory */
	virtual FText GetNodeCategory() const;

	/** [EN] Node name, used to query runtime state by name; returns the object name by default (subclasses can override, e.g. Objective returns NodeName). [CN] 节点名称，用于按名字查询运行状态等；默认返回对象名，子类可重写（如目标节点返回 NodeName） */
	virtual FName GetNodeName() const;

	/** [EN] Whether only one instance of this node is allowed in the same graph (true for the Start node). [CN] 该节点在同一张图里是否只允许存在一个（Start 节点为 true） */
	virtual bool IsUniqueNode() const;

	/** [EN] Input pin; null for the Start node. [CN] 输入引脚，Start 节点为空 */
	UEdGraphPin* GetInputPin() const;

	/** [EN] Output pin; null for the End node. [CN] 输出引脚，End 节点为空 */
	UEdGraphPin* GetOutputPin() const;

	/** [EN] The first successor node. [CN] 第一个后继节点 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode* GetNextNode() const;

	/** [EN] All successor nodes; multiple branches exist when the output pin connects to several (subclasses can override to choose which output to follow). [CN] 所有后继节点，输出引脚连了多个时会有多个分支（子类可重写以决定走哪个输出） */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	virtual void GetNextNodes(TArray<UTaskFlowNode*>& OutNextNodes) const;

	/**
	 * [EN] Entry point of node logic, called by UTaskFlowRunner.
	 * [CN] 节点逻辑入口，由 UTaskFlowRunner 调用。
	 * [EN] Synchronous nodes call Super::Execute(Runner) to finish automatically; asynchronous nodes call FinishExecution at the appropriate time.
	 * [CN] 同步节点调用 Super::Execute(Runner) 即可自动完成；异步节点在合适的时机调用 FinishExecution。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "TaskFlow")
	void Execute(UTaskFlowRunner* Runner);

	/** [EN] Notify the flow to continue; asynchronous nodes must call this exactly once when their logic ends. [CN] 通知流程继续往下走，异步节点必须在逻辑结束时调用一次 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void FinishExecution();

	/**
	 * [EN] Called by the Runner when the whole flow ends (reaches End / finishes / is stopped).
	 * Subclasses must abort pending async work and release external bindings; the base detaches the Runner.
	 * [CN] 整个流程结束（到达 End / 完成 / 被停止）时由 Runner 调用；子类必须终止未完成的异步工作并释放外部绑定，基类负责断开 Runner。
	 */
	virtual void OnFlowEnded();

	/** [EN] Internal entry called by the Runner to record the Runner and trigger Execute. [CN] 内部入口，由 Runner 调用，用于记录 Runner 并触发 Execute */
	void BeginExecution(UTaskFlowRunner* Runner);

	/** [EN] Get the Runner currently executing this node (valid only during Execute). [CN] 获取当前执行本节点的 Runner（仅在 Execute 期间有效） */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowRunner* GetRunner() const;

	/** [EN] Get the context (PlayerState, GameState, etc.) executing this node; may be null. [CN] 获取执行本节点的上下文（PlayerState 或 GameState 等），可能为空 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	AActor* GetContext() const;

	/** [EN] Get the PlayerState for a personal task (valid for personal tasks). [CN] 获取个人任务所属的 PlayerState（个人任务时有效） */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	APlayerState* GetPlayerStateOwner() const;

	/** [EN] Get the GameState for a global task (valid for global tasks). [CN] 获取全局任务所属的 GameState（全局任务时有效） */
	UFUNCTION(BlueprintPure, Category = "TaskFlow")
	AGameStateBase* GetGameStateOwner() const;

	//~ Begin Blackboard read/write (forwarded to the current Runner)
	//~ 黑板读写（转发到当前 Runner）
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
	/** [EN] Collect successor nodes from the given output pin. [CN] 从指定输出引脚收集后继节点 */
	void CollectNodesFromPin(const UEdGraphPin* Pin, TArray<UTaskFlowNode*>& OutNodes) const;

	/** [EN] The Runner currently executing this node. [CN] 当前正在执行本节点的 Runner */
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowRunner> ActiveRunner;

	/** [EN] Runtime state of the node. [CN] 节点运行时状态 */
	UPROPERTY(Transient)
	ETaskFlowNodeState NodeState = ETaskFlowNodeState::NotExecuted;
};
