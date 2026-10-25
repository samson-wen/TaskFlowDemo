#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TaskFlowAction.generated.h"

class UTaskFlowNode_Objective;

/**
 * 任务动作基类：UTaskFlowNode_Objective 的 Actions 数组里的一项。
 * 开发者继承本类（C++ 或蓝图）并实现 Execute 即可自定义动作。
 * - 同步动作：Execute 里直接调用 Finish()；
 * - 异步动作：Execute 里不调用 Finish()，等事情做完（定时器、回调等）再调用 Finish()。
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "任务动作"))
class TASKFLOWGRAPH_API UTaskFlowAction : public UObject
{
	GENERATED_BODY()

public:
	/** 动作显示名，默认取类名（去掉 TaskFlowAction_ 前缀） */
	virtual FText GetActionName() const;

	/** 执行动作，OwnerNode 为所属目标节点 */
	UFUNCTION(BlueprintNativeEvent, Category = "TaskFlow")
	void Execute(UTaskFlowNode_Objective* OwnerNode);

	/** 通知所属节点本动作已完成，继续执行下一个动作 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void Finish();

	/** 是否已完成 */
	bool IsFinished() const { return bFinished; }

	/** 内部：由目标节点调用，触发 Execute */
	void Start(UTaskFlowNode_Objective* InOwnerNode);

	/** 获取所属目标节点 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowNode_Objective* GetOwnerObjective() const { return OwningObjective.Get(); }

	//~ Begin 黑板读写（转发到所属目标节点）
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
	UPROPERTY(Transient)
	TObjectPtr<UTaskFlowNode_Objective> OwningObjective;

	UPROPERTY(Transient)
	bool bFinished = false;
};
