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
	 *  Optional default task flow started automatically on BeginPlay through the TaskFlowGraph plugin.
	 *  Leave null to disable auto-start. The TaskFlowSystem world subsystem drives execution.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TaskFlow")
	TObjectPtr<UTaskFlowAsset> DefaultTaskFlow;

	//~ Begin AGameModeBase Interface
	virtual void BeginPlay() override;
	//~ End AGameModeBase Interface
};



