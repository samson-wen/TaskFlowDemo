#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TaskFlowAsset.h"
#include "GSTaskFlowComponent.generated.h"

class UTaskFlowAsset;
class UTaskFlowRunner;

/**
 * Generic GameState component that starts a configurable list of task flows when the game begins.
 *
 * Add this component to a GameState (or a Blueprint subclass of one). Every asset in
 * DefaultTaskFlows is resolved and started in BeginPlay, with the owning GameState passed as the
 * flow context, so the runners are treated as global tasks (GameStateOwner is set).
 *
 * Assets are soft references: listing them here does not force-load them with the GameState class.
 * They are loaded synchronously when the component starts them.
 */
UCLASS(Blueprintable, ClassGroup = (TaskFlow), meta = (BlueprintSpawnableComponent, DisplayName = "GS Task Flow Component"))
class TASKFLOWDEMO_API UGSTaskFlowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGSTaskFlowComponent();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

	/** Task flow assets started automatically when this GameState begins play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow")
	TArray<TSoftObjectPtr<UTaskFlowAsset>> DefaultTaskFlows;

	/** Start one task flow with the owning GameState as its context. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	UTaskFlowRunner* StartTaskFlow(UTaskFlowAsset* InAsset);

	/** Load and start every asset listed in DefaultTaskFlows. */
	UFUNCTION(BlueprintCallable, Category = "TaskFlow")
	void StartDefaultTaskFlows();

private:
	/** Guards against re-registering the component starting the defaults twice. */
	bool bDefaultFlowsStarted = false;
};
