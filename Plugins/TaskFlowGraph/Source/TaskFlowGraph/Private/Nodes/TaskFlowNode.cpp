#include "Nodes/TaskFlowNode.h"

#include "EdGraph/EdGraphPin.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "TaskFlowGraphSchema.h"
#include "TaskFlowRunner.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode"

UTaskFlowNode::UTaskFlowNode()
{
}

void UTaskFlowNode::AllocateDefaultPins()
{
	UEdGraphPin* InputPin = CreatePin(EGPD_Input, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Input);
	InputPin->PinFriendlyName = LOCTEXT("TaskFlowNode_InputPin", "进入");

	UEdGraphPin* OutputPin = CreatePin(EGPD_Output, TaskFlowConstants::PinCategory, TaskFlowConstants::PinName_Output);
	OutputPin->PinFriendlyName = LOCTEXT("TaskFlowNode_OutputPin", "完成");
}

FText UTaskFlowNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FString Title = GetClass()->GetName();
	Title.RemoveFromStart(TEXT("TaskFlowNode_"));

	return FText::FromString(FName::NameToDisplayString(Title, false));
}

FLinearColor UTaskFlowNode::GetNodeTitleColor() const
{
	return GetStateColor();
}

FOnTaskFlowNodeStateChanged& UTaskFlowNode::OnNodeStateChanged()
{
	static FOnTaskFlowNodeStateChanged Delegate;
	return Delegate;
}

ETaskFlowNodeState UTaskFlowNode::GetNodeState() const
{
	return NodeState;
}

void UTaskFlowNode::SetNodeState(ETaskFlowNodeState InState)
{
	if (NodeState == InState)
	{
		return;
	}

	NodeState = InState;

	OnNodeStateChanged().Broadcast(this);
}

FLinearColor UTaskFlowNode::GetStateColor() const
{
	switch (NodeState)
	{
	case ETaskFlowNodeState::Executing:
		return FLinearColor(1.0f, 0.75f, 0.1f);   // 黄色
	case ETaskFlowNodeState::Succeeded:
		return FLinearColor(0.1f, 0.6f, 0.2f);    // 绿色
	case ETaskFlowNodeState::NotExecuted:
	default:
		return FLinearColor(0.45f, 0.45f, 0.45f); // 灰色
	}
}

FText UTaskFlowNode::GetTooltipText() const
{
	return GetNodeTitle(ENodeTitleType::ListView);
}

bool UTaskFlowNode::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* InSchema) const
{
	return Cast<UTaskFlowGraphSchema>(InSchema) != nullptr;
}

bool UTaskFlowNode::CanUserDeleteNode() const
{
	return !IsUniqueNode();
}

FText UTaskFlowNode::GetNodeCategory() const
{
	const FString MetaCategory = GetClass()->GetMetaData(TaskFlowConstants::MetaDataKey_NodeCategory);
	if (!MetaCategory.IsEmpty())
	{
		return FText::FromString(MetaCategory);
	}

	return LOCTEXT("TaskFlowNode_DefaultCategory", "通用");
}

FName UTaskFlowNode::GetNodeName() const
{
	return GetFName();
}

bool UTaskFlowNode::IsUniqueNode() const
{
	return false;
}

UEdGraphPin* UTaskFlowNode::GetInputPin() const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin && Pin->Direction == EGPD_Input)
		{
			return Pin;
		}
	}

	return nullptr;
}

UEdGraphPin* UTaskFlowNode::GetOutputPin() const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin && Pin->Direction == EGPD_Output)
		{
			return Pin;
		}
	}

	return nullptr;
}

UTaskFlowNode* UTaskFlowNode::GetNextNode() const
{
	TArray<UTaskFlowNode*> NextNodes;
	GetNextNodes(NextNodes);

	return NextNodes.Num() > 0 ? NextNodes[0] : nullptr;
}

void UTaskFlowNode::GetNextNodes(TArray<UTaskFlowNode*>& OutNextNodes) const
{
	OutNextNodes.Reset();
	CollectNodesFromPin(GetOutputPin(), OutNextNodes);
}

void UTaskFlowNode::CollectNodesFromPin(const UEdGraphPin* Pin, TArray<UTaskFlowNode*>& OutNodes) const
{
	if (Pin == nullptr)
	{
		return;
	}

	for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
	{
		if (LinkedPin == nullptr)
		{
			continue;
		}

		if (UTaskFlowNode* NextNode = Cast<UTaskFlowNode>(LinkedPin->GetOwningNode()))
		{
			OutNodes.Add(NextNode);
		}
	}
}

void UTaskFlowNode::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// 默认实现：执行完立即推进到下一个节点
	FinishExecution();
}

void UTaskFlowNode::FinishExecution()
{
	UTaskFlowRunner* Runner = ActiveRunner.Get();
	ActiveRunner = nullptr;

	if (Runner != nullptr)
	{
		Runner->NotifyNodeFinished(this);
	}
}

void UTaskFlowNode::BeginExecution(UTaskFlowRunner* Runner)
{
	ActiveRunner = Runner;
	Execute(Runner);
}

UTaskFlowRunner* UTaskFlowNode::GetRunner() const
{
	return ActiveRunner.Get();
}

AActor* UTaskFlowNode::GetContext() const
{
	UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr ? Runner->GetContext() : nullptr;
}

APlayerState* UTaskFlowNode::GetPlayerStateOwner() const
{
	UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr ? Runner->GetPlayerStateOwner() : nullptr;
}

AGameStateBase* UTaskFlowNode::GetGameStateOwner() const
{
	UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr ? Runner->GetGameStateOwner() : nullptr;
}

bool UTaskFlowNode::SetBlackboardInt(FName Key, int32 Value)
{
	UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->SetBlackboardInt(Key, Value);
}

bool UTaskFlowNode::GetBlackboardInt(FName Key, int32& OutValue) const
{
	const UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->GetBlackboardInt(Key, OutValue);
}

bool UTaskFlowNode::SetBlackboardFloat(FName Key, float Value)
{
	UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->SetBlackboardFloat(Key, Value);
}

bool UTaskFlowNode::GetBlackboardFloat(FName Key, float& OutValue) const
{
	const UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->GetBlackboardFloat(Key, OutValue);
}

bool UTaskFlowNode::SetBlackboardBool(FName Key, bool Value)
{
	UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->SetBlackboardBool(Key, Value);
}

bool UTaskFlowNode::GetBlackboardBool(FName Key, bool& OutValue) const
{
	const UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->GetBlackboardBool(Key, OutValue);
}

bool UTaskFlowNode::SetBlackboardName(FName Key, FName Value)
{
	UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->SetBlackboardName(Key, Value);
}

bool UTaskFlowNode::GetBlackboardName(FName Key, FName& OutValue) const
{
	const UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->GetBlackboardName(Key, OutValue);
}

bool UTaskFlowNode::SetBlackboardString(FName Key, const FString& Value)
{
	UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->SetBlackboardString(Key, Value);
}

bool UTaskFlowNode::GetBlackboardString(FName Key, FString& OutValue) const
{
	const UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->GetBlackboardString(Key, OutValue);
}

bool UTaskFlowNode::SetBlackboardObject(FName Key, UObject* Value)
{
	UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->SetBlackboardObject(Key, Value);
}

bool UTaskFlowNode::GetBlackboardObject(FName Key, UObject*& OutValue) const
{
	const UTaskFlowRunner* Runner = ActiveRunner.Get();
	return Runner != nullptr && Runner->GetBlackboardObject(Key, OutValue);
}

#undef LOCTEXT_NAMESPACE
