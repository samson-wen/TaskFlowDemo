#pragma once

#include "CoreMinimal.h"

class UTaskFlowGraph;

/** JSON serialization for the task flow graph (export / import nodes and connections). */
namespace TaskFlowGraphSerializer
{
	/** Export the graph to a JSON string; returns true on success. */
	bool ExportGraphToJson(UTaskFlowGraph* Graph, FString& OutJson);

	/** Import the graph from a JSON string (clears existing nodes); on failure OutError carries the error message. */
	bool ImportGraphFromJson(UTaskFlowGraph* Graph, const FString& InJson, FString& OutError);
}
