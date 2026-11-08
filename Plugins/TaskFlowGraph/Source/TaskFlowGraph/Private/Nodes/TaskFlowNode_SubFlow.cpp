#include "Nodes/TaskFlowNode_SubFlow.h"

#include "TaskFlowAsset.h"
#include "TaskFlowRunner.h"
#include "TaskFlowSystem.h"
#include "TaskFlowTypes.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "TaskFlowNode_SubFlow"

namespace
{
	void LogSubFlow(const FString& AssetName, const TCHAR* Event, const UTaskFlowNode* Node, const TCHAR* Detail)
	{
		LogTaskFlowLifecycle(AssetName, Event, Node->GetName(), *Node->GetClass()->GetName(), Detail);
	}
}

FText UTaskFlowNode_SubFlow::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const FString AssetName = SubFlowAsset.IsNull() ? FString(TEXT("<None>")) : SubFlowAsset.GetAssetName();
	return FText::FromString(FString::Printf(TEXT("Sub Flow: %s"), *AssetName));
}

FText UTaskFlowNode_SubFlow::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Runs another TaskFlowAsset as a sub-flow and waits for it to finish before continuing.");
}

#if WITH_EDITOR
void UTaskFlowNode_SubFlow::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// The title is generated from SubFlowAsset. Do not filter by property name: when a TSoftObjectPtr is assigned via the asset picker / drag-and-drop,
	//      the event may come from a property chain whose child name is not SubFlowAsset; rebuilding the widget on any node property change is safest.
	//      On the editor side, FTaskFlowGraphEditor::OnNodeStateChanged calls GraphEditor->RefreshNode(*Node),
	//      the same path used to refresh the title when an Objective node's NodeName changes.
	UTaskFlowNode::OnNodeStateChanged().Broadcast(this);
}
#endif

void UTaskFlowNode_SubFlow::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// The node may be executed several times from the same asset; reset runtime state first.
	bFlowEnded = false;
	SubRunner = nullptr;

	if (Runner == nullptr)
	{
		FinishExecution();
		return;
	}

	UTaskFlowAsset* Asset = SubFlowAsset.LoadSynchronous();
	if (Asset == nullptr)
	{
		// No asset configured: treat it as a no-op and let the parent flow continue.
		LogSubFlow(FString(TEXT("<None>")), TEXT("SubFlowRejected"), this, TEXT("SubFlowAsset is not set"));
		FinishExecution();
		return;
	}

	UTaskFlowSystem* System = Runner->GetTaskFlowSystem();
	if (System == nullptr)
	{
		LogSubFlow(Asset->GetName(), TEXT("SubFlowRejected"), this, TEXT("No TaskFlowSystem for the parent Runner"));
		FinishExecution();
		return;
	}

	// NodeState / ActiveRunner live on the shared asset: two Runners executing the same asset would overwrite each other,
	//      and the child's FinishFlow notifies every node in that asset via OnFlowEnded, detaching the parent node too. Reject this explicitly.
	if (Runner->Asset.Get() == Asset)
	{
		LogSubFlow(Asset->GetName(), TEXT("SubFlowRejected"), this, TEXT("Self-reference: sub flow equals parent asset"));
		FinishExecution();
		return;
	}

	// Reject any asset that is already running: this prevents loops such as A->B->A and concurrent sub-flows of the same asset.
	if (System->IsTaskFlowAssetRunning(Asset))
	{
		LogSubFlow(Asset->GetName(), TEXT("SubFlowRejected"), this, TEXT("Asset is already running (loop or concurrent sub flow)"));
		FinishExecution();
		return;
	}

	SubRunner = System->RunTaskFlow(Asset, Runner->GetContext());
	if (SubRunner == nullptr)
	{
		LogSubFlow(Asset->GetName(), TEXT("SubFlowRejected"), this, TEXT("RunTaskFlow failed"));
		FinishExecution();
		return;
	}

	// Critical timing: RunTaskFlow calls Start() synchronously; with StepDelaySeconds==0 and only synchronous nodes in the child
	//      (or an immediate End), OnFinished is broadcast before we can AddDynamic. Just complete in that case.
	if (!SubRunner->IsRunning())
	{
		LogSubFlow(Asset->GetName(), TEXT("SubFlowFinished"), this, TEXT("Child finished synchronously"));
		SubRunner = nullptr;
		FinishExecution();
		return;
	}

	SubRunner->OnFinished.AddDynamic(this, &UTaskFlowNode_SubFlow::HandleSubFlowFinished);
	LogSubFlow(Asset->GetName(), TEXT("SubFlowStarted"), this, TEXT("Waiting for child OnFinished"));
}

void UTaskFlowNode_SubFlow::HandleSubFlowFinished(UTaskFlowRunner* FinishedRunner)
{
	if (bFlowEnded)
	{
		return;
	}

	if (FinishedRunner != nullptr && FinishedRunner != SubRunner.Get())
	{
		return;
	}

	// Unbind before advancing to avoid reentrancy / duplicate callbacks caused by FinishExecution.
	if (SubRunner != nullptr)
	{
		SubRunner->OnFinished.RemoveAll(this);
	}

	const FString AssetName = SubFlowAsset.IsNull() ? FString(TEXT("<None>")) : SubFlowAsset.GetAssetName();
	LogSubFlow(AssetName, TEXT("SubFlowFinished"), this, TEXT("Child flow ended"));

	SubRunner = nullptr;
	FinishExecution();
}

void UTaskFlowNode_SubFlow::OnFlowEnded()
{
	// Parent flow ended (End / Stop): set the guard flag and unbind first, then stop the child flow.
	//      SubRunner->Stop() fires OnFinished again, which would call FinishExecution twice if we had not unbound first.
	bFlowEnded = true;

	if (SubRunner != nullptr)
	{
		SubRunner->OnFinished.RemoveAll(this);

		if (SubRunner->IsRunning())
		{
			SubRunner->Stop();
		}

		SubRunner = nullptr;
	}

	Super::OnFlowEnded();
}

#undef LOCTEXT_NAMESPACE
