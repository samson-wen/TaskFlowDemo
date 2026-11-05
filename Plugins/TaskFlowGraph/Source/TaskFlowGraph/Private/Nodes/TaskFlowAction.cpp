#include "Nodes/TaskFlowAction.h"

#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowRunner.h"
#include "TaskFlowSystem.h"

FText UTaskFlowAction::GetActionName() const
{
	FString Name = GetClass()->GetName();
	Name.RemoveFromStart(TEXT("TaskFlowAction_"));

	return FText::FromString(FName::NameToDisplayString(Name, false));
}

void UTaskFlowAction::Execute_Implementation(UTaskFlowNode_Objective* OwnerNode)
{
	// [EN] Default implementation: finish synchronously.
	// [CN] 默认实现：同步完成
	Finish();
}

void UTaskFlowAction::Finish()
{
	if (bFinished)
	{
		return;
	}

	bFinished = true;

	if (OwningObjective.Get() != nullptr)
	{
		OwningObjective->OnActionFinished(this);
	}
}

void UTaskFlowAction::Start(UTaskFlowNode_Objective* InOwnerNode)
{
	OwningObjective = InOwnerNode;
	bFinished = false;

	Execute(InOwnerNode);
}

bool UTaskFlowAction::SetBlackboardInt(FName Key, int32 Value)
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->SetBlackboardInt(Key, Value);
}

bool UTaskFlowAction::GetBlackboardInt(FName Key, int32& OutValue) const
{
	const UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->GetBlackboardInt(Key, OutValue);
}

bool UTaskFlowAction::SetBlackboardFloat(FName Key, float Value)
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->SetBlackboardFloat(Key, Value);
}

bool UTaskFlowAction::GetBlackboardFloat(FName Key, float& OutValue) const
{
	const UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->GetBlackboardFloat(Key, OutValue);
}

bool UTaskFlowAction::SetBlackboardBool(FName Key, bool Value)
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->SetBlackboardBool(Key, Value);
}

bool UTaskFlowAction::GetBlackboardBool(FName Key, bool& OutValue) const
{
	const UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->GetBlackboardBool(Key, OutValue);
}

bool UTaskFlowAction::SetBlackboardName(FName Key, FName Value)
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->SetBlackboardName(Key, Value);
}

bool UTaskFlowAction::GetBlackboardName(FName Key, FName& OutValue) const
{
	const UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->GetBlackboardName(Key, OutValue);
}

bool UTaskFlowAction::SetBlackboardString(FName Key, const FString& Value)
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->SetBlackboardString(Key, Value);
}

bool UTaskFlowAction::GetBlackboardString(FName Key, FString& OutValue) const
{
	const UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->GetBlackboardString(Key, OutValue);
}

bool UTaskFlowAction::SetBlackboardObject(FName Key, UObject* Value)
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->SetBlackboardObject(Key, Value);
}

bool UTaskFlowAction::GetBlackboardObject(FName Key, UObject*& OutValue) const
{
	const UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr && Owner->GetBlackboardObject(Key, OutValue);
}

void UTaskFlowAction::SetObjectiveProgress(int32 Value)
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	if (Owner != nullptr)
	{
		Owner->SetProgress(Value);
	}
}

void UTaskFlowAction::AddObjectiveProgress(int32 Amount)
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	if (Owner != nullptr)
	{
		Owner->AddProgress(Amount);
	}
}

AActor* UTaskFlowAction::GetContext() const
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr ? Owner->GetContext() : nullptr;
}

APlayerState* UTaskFlowAction::GetPlayerStateOwner() const
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr ? Owner->GetPlayerStateOwner() : nullptr;
}

AGameStateBase* UTaskFlowAction::GetGameStateOwner() const
{
	UTaskFlowNode_Objective* Owner = OwningObjective.Get();
	return Owner != nullptr ? Owner->GetGameStateOwner() : nullptr;
}

UTaskFlowSystem* UTaskFlowAction::GetTaskFlowSystem() const
{
	// [CN] 优先经由所属目标节点当前执行的 Runner 取得系统（最可靠）。
	if (UTaskFlowNode_Objective* Owner = OwningObjective.Get())
	{
		if (UTaskFlowRunner* Runner = Owner->GetRunner())
		{
			if (UTaskFlowSystem* System = Runner->GetTaskFlowSystem())
			{
				return System;
			}
		}
	}

	// [CN] 回退：通过动作对象的 World 上下文直接取世界子系统。
	return UTaskFlowSystem::Get(this);
}

void UTaskFlowAction::ReceiveTaskFlowEvent(const FTaskFlowEvent& Event)
{
	// [EN] No handling by default; override in subclasses.
	// [CN] 默认不处理，子类重写
}
