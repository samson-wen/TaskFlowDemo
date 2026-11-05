#pragma once

#include "CoreMinimal.h"

/** [EN] Log category for all task flow runtime logs (node lifecycle, actions, diagnostics). [CN] 任务流运行期日志类别（节点生命周期、动作、诊断信息） */
TASKFLOWGRAPH_API DECLARE_LOG_CATEGORY_EXTERN(LogTaskFlow, Log, All);

/**
 * [EN] Shared formatter for every TaskFlow lifecycle log line; guarantees a uniform layout everywhere.
 * [CN] 所有 TaskFlow 生命周期日志共用的格式化入口，保证各处格式完全一致。
 * [EN] Format: [TaskFlow] Flow=<Asset> | Event=<Event> | Node=<Name> | Class=<Class> | Detail=<Detail>
 */
FORCEINLINE void LogTaskFlowLifecycle(const FString& FlowName, const TCHAR* Event, const FString& NodeName, const TCHAR* ClassName, const TCHAR* Detail)
{
	UE_LOG(LogTaskFlow, Log, TEXT("[TaskFlow] Flow=%s | Event=%s | Node=%s | Class=%s | Detail=%s"),
		*FlowName, Event, *NodeName, ClassName, Detail);
}

/** [EN] Shared constants used by the task flow graph. [CN] 任务流图使用的公共常量 */
namespace TaskFlowConstants
{
	/** [EN] Custom pin category, to avoid confusion with Blueprint K2 exec pins. [CN] 自定义引脚类型，避免和蓝图 K2 的 exec 引脚混淆 */
	const FName PinCategory(TEXT("TaskFlow"));

	/** [EN] Input pin name. [CN] 输入引脚名 */
	const FName PinName_Input(TEXT("In"));
	/** [EN] Output pin name. [CN] 输出引脚名 */
	const FName PinName_Output(TEXT("Out"));
	/** [EN] Output pin name for the success branch of a condition. [CN] 条件分支成功输出引脚名 */
	const FName PinName_Success(TEXT("Success"));
	/** [EN] Output pin name for the failure branch of a condition. [CN] 条件分支失败输出引脚名 */
	const FName PinName_Failed(TEXT("Failed"));

	/** [EN] UCLASS metadata Key for node category: UCLASS(meta = (TaskFlowCategory = "MyCategory")). [CN] 节点分类的 UCLASS 元数据 Key：UCLASS(meta = (TaskFlowCategory = "我的分类")) */
	const FName MetaDataKey_NodeCategory(TEXT("TaskFlowCategory"));

	/** [EN] Standard event name broadcast by the Runner when a task flow starts. [CN] 任务流程开始时由 Runner 广播的标准事件名 */
	const FName EventName_Started(TEXT("TaskFlowStarted"));

	/** [EN] Standard event name broadcast by the Runner when a task flow finishes (completed or aborted). [CN] 任务流程结束时（完成或中断）由 Runner 广播的标准事件名 */
	const FName EventName_Finished(TEXT("TaskFlowFinished"));
}
