// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CombatGameState.generated.h"

/**
 *  Simple GameState for a third person combat game
 */
UCLASS(abstract)
class ACombatGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	ACombatGameState();

protected:

	/** Initialization */
	virtual void BeginPlay() override;

};
