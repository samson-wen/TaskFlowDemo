#pragma once

#include "CoreMinimal.h"

class UTaskFlowGraph;

/** [EN] JSON serialization for the task flow graph (export / import nodes and connections). [CN] 任务流程图 Json 序列化（导出 / 导入节点与连接） */
namespace TaskFlowGraphSerializer
{
	/** [EN] Export the graph to a JSON string; returns true on success. [CN] 把图导出为 Json 字符串，成功返回 true */
	bool ExportGraphToJson(UTaskFlowGraph* Graph, FString& OutJson);

	/** [EN] Import the graph from a JSON string (clears existing nodes); on failure OutError carries the error message. [CN] 从 Json 字符串导入图（会清空现有节点），失败时 OutError 带错误信息 */
	bool ImportGraphFromJson(UTaskFlowGraph* Graph, const FString& InJson, FString& OutError);
}
