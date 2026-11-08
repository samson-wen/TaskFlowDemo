// Copyright Epic Games, Inc. All Rights Reserved.

#include "TaskFlowDemoGameMode.h"

#include "TaskFlowSystem.h"

ATaskFlowDemoGameMode::ATaskFlowDemoGameMode()
{
	// stub
}

void ATaskFlowDemoGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Demo of TaskFlowGraph plugin usage: start the configured default task flow.
	if (DefaultTaskFlow != nullptr)
	{
		if (UTaskFlowSystem* System = GetWorld()->GetSubsystem<UTaskFlowSystem>())
		{
			System->RunTaskFlow(DefaultTaskFlow, nullptr);
		}
	}
}
