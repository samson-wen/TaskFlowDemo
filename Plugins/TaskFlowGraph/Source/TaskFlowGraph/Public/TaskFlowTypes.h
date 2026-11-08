#pragma once

#include "CoreMinimal.h"

/** Log category for all task flow runtime logs (node lifecycle, actions, diagnostics). */
TASKFLOWGRAPH_API DECLARE_LOG_CATEGORY_EXTERN(LogTaskFlow, Log, All);

/**
 * Shared formatter for every TaskFlow lifecycle log line; guarantees a uniform layout everywhere.
 * Format: [TaskFlow] Flow=<Asset> | Event=<Event> | Node=<Name> | Class=<Class> | Detail=<Detail>
 */
FORCEINLINE void LogTaskFlowLifecycle(const FString& FlowName, const TCHAR* Event, const FString& NodeName, const TCHAR* ClassName, const TCHAR* Detail)
{
	UE_LOG(LogTaskFlow, Log, TEXT("[TaskFlow] Flow=%s | Event=%s | Node=%s | Class=%s | Detail=%s"),
		*FlowName, Event, *NodeName, ClassName, Detail);
}

/** Shared constants used by the task flow graph. */
namespace TaskFlowConstants
{
	/** Custom pin category, to avoid confusion with Blueprint K2 exec pins. */
	const FName PinCategory(TEXT("TaskFlow"));

	/** Input pin name. */
	const FName PinName_Input(TEXT("In"));
	/** Output pin name. */
	const FName PinName_Output(TEXT("Out"));
	/** Output pin name for the success branch of a condition. */
	const FName PinName_Success(TEXT("Success"));
	/** Output pin name for the failure branch of a condition. */
	const FName PinName_Failed(TEXT("Failed"));

	/** UCLASS metadata Key for node category: UCLASS(meta = (TaskFlowCategory = "MyCategory")). */
	const FName MetaDataKey_NodeCategory(TEXT("TaskFlowCategory"));

	/** Broadcast by the Start node when it executes, marking the beginning of a task flow. */
	const FName EventName_Started(TEXT("TaskFlowStarted"));

	/** Broadcast when a task flow completes normally (reached End or all branches finished). Never broadcast on Stop. */
	const FName EventName_Completed(TEXT("TaskFlowCompleted"));

	/** Broadcast by the Runner whenever a task flow finishes (completed or aborted). */
	const FName EventName_Finished(TEXT("TaskFlowFinished"));
}
