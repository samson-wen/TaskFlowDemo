#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameTaskFlowStatics.generated.h"

UCLASS()
class UGameTaskFlowStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Broadcast EVENT_ENEMY_DEATH to the task flow system. The killer may be null (environmental damage).
	 * Call this when an enemy dies; AIID is read from the dead enemy itself.
	 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Event")
	static void BroadcastKillTargetTaskFlow(ACharacter* InKillerCharacter, ACharacter* InDeathCharacter);

	/**
	 * Broadcast EVENT_TASK_ACCEPTED so AcceptTask Objectives can react to a player accepting a task.
	 * Call this from the task UI / task component after the task has been added to the player's list.
	 * InAcceptor is optional and is forwarded as FTaskFlowEvent::Sender (e.g. the PlayerState).
	 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Event", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void BroadcastTaskAccepted(const UObject* WorldContextObject, int32 TaskID, UObject* InAcceptor = nullptr);
};