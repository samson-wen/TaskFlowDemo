#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "EdGraph/EdGraphNode.h"
#include "TaskFlowNode.generated.h"

class UEdGraphPin;
class UTaskFlowGraphSchema;
class UTaskFlowNode;
class UTaskFlowRunner;

/** 任务流节点运行时状态 */
UENUM(BlueprintType)
enum class ETaskFlowNodeState : uint8
{
	/** 未执行 */
	NotExecuted UMETA(DisplayName = "未执行"),
	/** 正在执行 */
	Executing   UMETA(DisplayName = "执行中"),
	/** 执行成功 */
	Succeeded   UMETA(DisplayName = "已成功")
};

/** 节点状态变化广播（编辑器用它在运行时刷新节点颜色） */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTaskFlowNodeStateChanged, UTaskFlowNode*);

/**
 * 任务流节点基类。
 *
 * 开发者继承本类（C++ 或蓝图）并实现 Execute 即可定义自己的任务节点。
 * - 同步节点：在 Execute 里直接做完事情，基类实现会自动调用 FinishExecution 推进流程；
 * - 异步节点：Execute 里不调用基类实现，等事情做完（定时器、回调等）再手动调用 FinishExecution。
 *
 * 节点默认拥有一个输入引脚和一个输出引脚，可在图中自由连线。
 * Start 节点没有输入引脚，End 节点没有输出引脚。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, meta = (DisplayName = "任务节点"))
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
	//~ End UEdGraphNode Interface

	/** 节点状态变化广播 */
	static FOnTaskFlowNodeStateChanged& OnNodeStateChanged();

	/** 获取当前节点状态 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	ETaskFlowNodeState GetNodeState() const;

	/** 设置节点状态（由 Runner 调用） */
	void SetNodeState(ETaskFlowNodeState InState);

	/** 根据状态返回节点标题颜色：灰色=未执行，黄色=执行中，绿色=成功 */
	virtual FLinearColor GetStateColor() const;

	/** 右键菜单中的分类，默认读取 UCLASS 元数据 TaskFlowCategory */
	virtual FText GetNodeCategory() const;

	/** 该节点在同一张图里是否只允许存在一个（Start 节点为 true） */
	virtual bool IsUniqueNode() const;

	/** 输入引脚，Start 节点为空 */
	UEdGraphPin* GetInputPin() const;

	/** 输出引脚，End 节点为空 */
	UEdGraphPin* GetOutputPin() const;

	/** 第一个后继节点 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode* GetNextNode() const;

	/** 所有后继节点，输出引脚连了多个时会有多个分支（子类可重写以决定走哪个输出） */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	virtual void GetNextNodes(TArray<UTaskFlowNode*>& OutNextNodes) const;

	/**
	 * 节点逻辑入口，由 UTaskFlowRunner 调用。
	 * 同步节点调用 Super::Execute(Runner) 即可自动完成；异步节点在合适的时机调用 FinishExecution。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "TaskFlow")
	void Execute(UTaskFlowRunner* Runner);

	/** 通知流程继续往下走，异步节点必须在逻辑结束时调用一次 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void FinishExecution();

	/** 内部入口，由 Runner 调用，用于记录 Runner 并触发 Execute */
	void BeginExecution(UTaskFlowRunner* Runner);

	/** 获取当前执行本节点的 Runner（仅在 Execute 期间有效） */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowRunner* GetRunner() const;

	//~ Begin 黑板读写（转发到当前 Runner）
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
	//~ End 黑板读写

protected:
	/** 从指定输出引脚收集后继节点 */
	void CollectNodesFromPin(const UEdGraphPin* Pin, TArray<UTaskFlowNode*>& OutNodes) const;

	/** 当前正在执行本节点的 Runner */
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowRunner> ActiveRunner;

	/** 节点运行时状态 */
	UPROPERTY(Transient)
	ETaskFlowNodeState NodeState = ETaskFlowNodeState::NotExecuted;
};
