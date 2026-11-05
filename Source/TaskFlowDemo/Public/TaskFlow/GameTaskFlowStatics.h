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
	 * [EN] Broadcast EVENT_ENEMY_DEATH to the task flow system. The killer may be null (environmental damage).
	 * Call this when an enemy dies; AIID is read from the dead enemy itself.
	 * [CN] 向任务流系统广播 EVENT_ENEMY_DEATH。击杀者可以为空（环境伤害）。
	 * 敌人死亡时调用；AIID 取自死亡敌人本体。
	 */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow|Event")
	static void BroadcastKillTargetTaskFlow(ACharacter* InKillerCharacter, ACharacter* InDeathCharacter);
};