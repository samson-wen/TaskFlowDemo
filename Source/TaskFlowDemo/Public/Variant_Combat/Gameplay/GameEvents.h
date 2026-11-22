#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "GameEvents.generated.h"


/** Dynamic multicast delegate for global game events (Blueprint-bindable). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAcceptTaskEvent, int32, TaskId, APlayerState*, PlayerState);

/**
 * Game-instance-wide event bus for global broadcasting and listening.
 *
 * There is one instance per GameInstance: it survives level travel / seamless travel and is shared
 * by every World in that game session, so broadcasters and listeners always reach the same bus.
 * Unlike a UWorldSubsystem, it does not create a separate editor / PIE instance per World.
 *
 * C++:
 *   UGameEvents::Get(this)->OnGameEvent.AddDynamic(this, &UMyClass::HandleGameEvent);
 *   UGameEvents::Get(this)->BroadcastEvent(FName(TEXT("EnemyDied")), this);
 *
 * Blueprint: use the "Get Game Events" node, then "Assign On Game Event" and filter by EventName.
 * The engine's standard "Get Game Instance Subsystem" node also resolves this class.
 *
 * This bus is independent of UTaskFlowSystem::OnTaskFlowEvent; bridge them explicitly if task flow
 * events should also be observable here. For a process-wide bus shared across PIE clients / editor,
 * derive from UEngineSubsystem instead.
 */
UCLASS(BlueprintType)
class TASKFLOWDEMO_API UGameEvents : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Get the event bus for the GameInstance owning WorldContextObject; nullptr outside a running game. */
	UFUNCTION(BlueprintPure, Category = "Game Events", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Get Game Events"))
	static UGameEvents* Get(const UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FAcceptTaskEvent OnAcceptTaskEvt;
};
