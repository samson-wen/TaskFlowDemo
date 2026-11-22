// Copyright Epic Games, Inc. All Rights Reserved.

#include "GSTaskFlowComponent.h"

#include "TaskFlowRunner.h"
#include "TaskFlowSystem.h"
#include "TaskFlowTypes.h"

UGSTaskFlowComponent::UGSTaskFlowComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGSTaskFlowComponent::BeginPlay()
{
	Super::BeginPlay();
	StartDefaultTaskFlows();
}

UTaskFlowRunner* UGSTaskFlowComponent::StartTaskFlow(UTaskFlowAsset* InAsset)
{
	if (InAsset == nullptr)
	{
		return nullptr;
	}

	UTaskFlowSystem* System = UTaskFlowSystem::Get(this);
	if (System == nullptr)
	{
		UE_LOG(LogTaskFlow, Warning,
			TEXT("GSTaskFlowComponent: no TaskFlowSystem for %s."), *GetNameSafe(GetOwner()));
		return nullptr;
	}

	// The owning GameState is the context, so the runner treats this as a global task.
	UTaskFlowRunner* Runner = System->RunTaskFlow(InAsset, GetOwner());

	UE_LOG(LogTaskFlow, Log,
		TEXT("GSTaskFlowComponent: started %s for %s."),
		*InAsset->GetName(), *GetNameSafe(GetOwner()));

	return Runner;
}

void UGSTaskFlowComponent::StartDefaultTaskFlows()
{
	if (bDefaultFlowsStarted)
	{
		return;
	}
	bDefaultFlowsStarted = true;

	for (const TSoftObjectPtr<UTaskFlowAsset>& AssetRef : DefaultTaskFlows)
	{
		if (AssetRef.IsNull())
		{
			continue;
		}

		UTaskFlowAsset* Asset = AssetRef.LoadSynchronous();
		if (Asset == nullptr)
		{
			UE_LOG(LogTaskFlow, Warning,
				TEXT("GSTaskFlowComponent: failed to load asset %s."), *AssetRef.ToSoftObjectPath().ToString());
			continue;
		}

		StartTaskFlow(Asset);
	}
}
