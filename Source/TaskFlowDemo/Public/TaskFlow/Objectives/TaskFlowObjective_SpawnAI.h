#pragma once

#include "CoreMinimal.h"
#include "TaskFlowActorReference.h"
#include "TimerManager.h"
#include "Nodes/TaskFlowNode_Objective.h"

#include "TaskFlowObjective_SpawnAI.generated.h"

class ACombatEnemy;
class UTaskFlowRunner;
class UWorld;

/**
 * Specialized Objective node that spawns a group of AI enemies and completes once they are created.
 *
 * EnemyClass selects the (Blueprint) subclass of ACombatEnemy to spawn. Spawning follows the same
 * pattern as ACombatEnemySpawner::SpawnEnemy: SpawnActor with AdjustIfPossibleButAlwaysSpawn.
 * SpawnNum is copied into MaxProgress when the node executes, so Details-panel edits take effect.
 *
 * The spawn center is SpawnPointRef when it resolves to a level Actor, otherwise SpawnPosition;
 * it is treated as a ground point, and spawned characters are lifted by their capsule half height.
 * A single AI spawns at the center. When SpawnNum > 1, each AI is placed at a random reachable
 * point on the NavMesh within SpawnRange of the center. With no navigation data (or no reachable
 * point) it falls back to a random offset inside the same radius, so a level without a
 * NavMeshBoundsVolume still produces scattered AI instead of one overlapping stack.
 *
 * Each spawned enemy receives AIID (for EVENT_ENEMY_DEATH / KillTarget matching), and AITag, when
 * set, is added to its Actor tags.
 *
 * Spawning is spread across frames: SpawnPerFrame enemies are created per tick via
 * SetTimerForNextTick, so a large SpawnNum does not cause a single-frame hitch. The Objective stays
 * executing until the last spawn and then completes through one SetProgress(MaxProgress), so a
 * failed spawn never leaves the flow stuck. If the Objective has asynchronous Actions, the base
 * class also waits for those before completing. Pending spawn ticks are cancelled in OnFlowEnded.
 *
 * Listening is enabled in the constructor so events arriving while the Objective executes are
 * forwarded to its Actions via UTaskFlowAction::ReceiveTaskFlowEvent; the base class owns the
 * delegate binding / unbinding (cached System) and the flow-ended guard.
 */
UCLASS(Blueprintable, meta = (DisplayName = "SpawnAI Objective", TaskFlowCategory = "Task"))
class TASKFLOWDEMO_API UTaskFlowObjective_SpawnAI : public UTaskFlowNode_Objective
{
	GENERATED_BODY()

public:
	UTaskFlowObjective_SpawnAI();

	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;

public:
	/** Concrete enemy class to spawn (normally a Blueprint subclass of ACombatEnemy). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnAI")
	TSubclassOf<ACombatEnemy> EnemyClass;

	/** Identifier assigned to every spawned enemy; 0 means unassigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnAI")
	int64 AIID = 0;

	/** Optional tag added to every spawned enemy's Actor tags. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnAI")
	FString AITag = "";

	/** Number of AI to spawn. Copied into MaxProgress when the node executes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnAI", meta = (ClampMin = "1"))
	int32 SpawnNum = 1;

	/** Optional level Actor used as the spawn center; takes priority over SpawnPosition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnAI")
	FTaskFlowActorReference SpawnPointRef;

	/** Spawn center used when SpawnPointRef is not configured or cannot be resolved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnAI")
	FVector SpawnPosition;

	/** Radius around the spawn center used to pick random navigable points when spawning multiple AI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnAI", meta = (ClampMin = "0.0", Units = "cm"))
	float SpawnRange = 300.0f;

	/** Number of AI to spawn per frame. 1 spreads the work as widely as possible; raise it to finish sooner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnAI", meta = (ClampMin = "1"))
	int32 SpawnPerFrame = 1;

protected:
	/** Flow ended: cancel pending spawn ticks before the base class detaches the Runner. */
	virtual void OnFlowEnded() override;

private:
	/** Spawn up to SpawnPerFrame enemies, then queue the next tick or complete the Objective. */
	void HandleSpawnTick();

	/** Queue one spawn callback for the next frame. */
	void ScheduleNextSpawn();

	/** Cancel the pending spawn callback. */
	void ClearSpawnTimer();

	/** Clear spawn state and drive the Objective to completion exactly once. */
	void FinishSpawning();

	/** World owning the spawn timer; weak so a destroyed World cannot leave a dangling timer. */
	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> SpawnWorld;

	/** Spawn parameters cached at execution time and shared by every tick. */
	FVector CachedSpawnCenter = FVector::ZeroVector;
	FRotator CachedSpawnRotation = FRotator::ZeroRotator;
	float CachedCapsuleHalfHeight = 88.0f;

	int32 PendingSpawnCount = 0;
	int32 SpawnedCount = 0;
	bool bSpawnCancelled = false;
	FTimerHandle SpawnTimerHandle;
};
