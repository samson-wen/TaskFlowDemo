// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TaskFlowAsset.h"
#include "TaskFlowDemoGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class ATaskFlowDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	ATaskFlowDemoGameMode();

	/**
	 *  [EN] Optional default task flow started automatically on BeginPlay through the TaskFlowGraph plugin.
	 *  [CN] 可选：在 BeginPlay 时通过 TaskFlowGraph 插件自动启动的默认任务流程。
	 *  [EN] Leave null to disable auto-start. The TaskFlowSystem world subsystem drives execution.
	 *  [CN] 留空则不自动启动；由 TaskFlowSystem 世界子系统驱动执行。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<UTaskFlowAsset> DefaultTaskFlow;

	//~ Begin AGameModeBase Interface
	virtual void BeginPlay() override;
	//~ End AGameModeBase Interface
};



