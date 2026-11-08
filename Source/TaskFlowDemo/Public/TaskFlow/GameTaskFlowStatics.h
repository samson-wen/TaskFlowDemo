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
};