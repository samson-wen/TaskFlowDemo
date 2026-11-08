#pragma once

#include "CoreMinimal.h"
#include "TaskFlowEvent.h"
#include "TaskFlowAsset.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPtr.h"
#include "GameTaskFlowTypes.generated.h"


namespace GameTaskFlowEvents
{
	
	const FName EVENT_TASKFLOW_VOLUME_ENTER(TEXT("EVENT_TASKFLOW_VOLUME_ENTER"));
	const FName EVENT_TASKFLOW_VOLUME_LEAVE(TEXT("EVENT_TASKFLOW_VOLUME_LEAVE"));

	const FName EVENT_ENEMY_DEATH(TEXT("EVENT_ENEMY_DEATH"));

}

/**
 * Payload for volume enter/leave events, stored in FTaskFlowEvent::EventData.
 *
 * Deliberately NOT derived from FTaskFlowEvent: it is a standalone payload, and the event metadata
 * (EventName / Sender / Payload) lives on the outer FTaskFlowEvent.
 */
USTRUCT(BlueprintType)
struct FTaskFlowVolumeEvent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	int32 VolumeID = 0;

	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	FString VolumeTag = "";
};


/**
 * Payload for EVENT_ENEMY_DEATH, stored in FTaskFlowEvent::EventData.
 */
USTRUCT(BlueprintType)
struct FTaskFlowKillTargetEvent
{
	GENERATED_BODY()

public:
	/** Task-flow identifier of the dead enemy; 0 means unassigned. */
	UPROPERTY(BlueprintReadWrite, Category = "TaskFlow")
	int64 AIID = 0;
};

/**
 * A single objective inside a game task.
 *
 * RequiredProgress is the configured count to reach (e.g. kill 3 enemies); CurrentProgress is
 * runtime-only state and is not serialized into the asset / DataTable.
 * It is not serialized into the asset / DataTable.
 */
USTRUCT(BlueprintType)
struct FGameTaskObjective
{
	GENERATED_BODY()

public:
	/** Objective identifier, unique within its owning task; 0 means unassigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Objective")
	int32 ObjectiveID = 0;

	/** Objective content / description shown to the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Objective", meta = (MultiLine = true))
	FText Content;

	/** Progress count required to complete this objective; 0 means a single trigger completes it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Objective", meta = (ClampMin = "0"))
	int32 RequiredProgress = 1;

	/** Current progress at runtime (not saved). */
	UPROPERTY(BlueprintReadOnly, Category = "Task|Objective", Transient)
	int32 CurrentProgress = 0;
};

/**
 * Game task definition used as a UDataTable row struct (derives from FTableRowBase).
 *
 * Holds static configuration (identity, text, objective list) plus the TaskFlowAsset that drives
 * execution. The DataTable's RowName is the primary key for lookups.
 *
 * NOTE: row data is shared by every reader of the asset. FGameTaskObjective::CurrentProgress is
 * Transient (never saved), but still must not be used as per-player runtime state - keep that in a
 * separate runtime struct owned by the player / task instance.
 * but it must not be used as per-player runtime state; such state belongs in a separate runtime struct owned by the player / task instance.
 */
USTRUCT(BlueprintType)
struct FGameTask : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** Task identifier, unique across the task set; 0 means unassigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	int32 TaskID = 0;

	/** Task title. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FText Title;

	/** Task content / description. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (MultiLine = true))
	FText Content;

	/** Ordered list of objectives for this task. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	TArray<FGameTaskObjective> Objectives;

	/**
	 * TaskFlowAsset bound to this task; drives the actual flow at runtime.
	 *
	 * Soft reference so a DataTable does not force-load every flow asset it mentions. Resolve with
	 * LoadSynchronous() (or an async load) before passing it to UTaskFlowSystem::RunTaskFlow().
	 * Resolve it with LoadSynchronous() (or an async load) before passing it to UTaskFlowSystem::RunTaskFlow().
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	TSoftObjectPtr<UTaskFlowAsset> TaskFlowAsset;
};

