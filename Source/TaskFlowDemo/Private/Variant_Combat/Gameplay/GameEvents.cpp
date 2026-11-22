// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameEvents.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

UGameEvents* UGameEvents::Get(const UObject* WorldContextObject)
{
	UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
	return GameInstance != nullptr ? GameInstance->GetSubsystem<UGameEvents>() : nullptr;
}