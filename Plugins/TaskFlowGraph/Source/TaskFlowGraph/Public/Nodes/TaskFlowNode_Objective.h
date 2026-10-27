#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode.h"
#include "TaskFlowNode_Objective.generated.h"

class UTaskFlowAction;
class UTaskFlowRunner;

/**
 * 目标节点：表示任务流程中的一个目标 / 步骤。
 * 它继承自 UTaskFlowNode，可以直接在 TaskFlowGraph 资产编辑器里创建并连线；
 * 节点被触发后，会按顺序依次执行 Actions 列表里的动作，全部完成后推进到下一个节点。
 */
UCLASS(Blueprintable, meta = (DisplayName = "目标", TaskFlowCategory = "任务"))
class TASKFLOWGRAPH_API UTaskFlowNode_Objective : public UTaskFlowNode
{
	GENERATED_BODY()

public:
	UTaskFlowNode_Objective();

	//~ Begin UEdGraphNode Interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FName GetNodeName() const override;
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;

	/** 目标内容描述，会显示在节点的悬浮提示和 Details 面板里 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (MultiLine = true))
	FText ObjectiveDescription;

	/** 节点名称，同一资产内不可重复（留空表示未命名，不校验） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName NodeName;

	//~ Begin UObject Interface
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

	/** 该目标要执行的动作列表，按顺序依次执行 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", Instanced)
	TArray<TObjectPtr<UTaskFlowAction>> Actions;

	/** 最大进度，0 表示不需要进度；大于 0 时需当前进度达到该值才算完成 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective", meta = (ClampMin = "0"))
	int32 MaxProgress = 0;

	/** 当前进度（运行时），达到 MaxProgress 后目标才算完成 */
	UPROPERTY(BlueprintReadOnly, Category = "Objective", Transient)
	int32 CurrentProgress = 0;

	/** 增加当前进度 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void AddProgress(int32 Amount);

	/** 设置当前进度 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetProgress(int32 Value);

	/** 内部：某个动作完成时回调，继续执行下一个动作 */
	void OnActionFinished(UTaskFlowAction* Action);

private:
	/** 判断当前 NodeName 是否与图中其他目标节点重复 */
	bool IsNodeNameDuplicate() const;

	/** 继续执行下一个动作 */
	void ExecuteNextAction();

	/** 动作全部完成且进度达标时，调用 FinishExecution 完成节点 */
	void TryComplete();

	/** 当前正在执行（或等待完成）的动作索引 */
	int32 CurrentActionIndex = 0;

	/** 防止同步动作里 Finish 造成的递归 */
	bool bAdvancingActions = false;

	/** 动作是否已全部执行完 */
	bool bActionsCompleted = false;

	/** 编辑前的 NodeName，重复时用于回退 */
	FName OldNodeName;
};
