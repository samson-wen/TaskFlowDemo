#pragma once

#include "CoreMinimal.h"

/** 任务流图使用的公共常量 */
namespace TaskFlowConstants
{
	/** 自定义引脚类型，避免和蓝图 K2 的 exec 引脚混淆 */
	const FName PinCategory(TEXT("TaskFlow"));

	/** 输入引脚名 */
	const FName PinName_Input(TEXT("In"));
	/** 输出引脚名 */
	const FName PinName_Output(TEXT("Out"));
	/** 条件分支成功输出引脚名 */
	const FName PinName_Success(TEXT("Success"));
	/** 条件分支失败输出引脚名 */
	const FName PinName_Failed(TEXT("Failed"));

	/** 节点分类的 UCLASS 元数据 Key：UCLASS(meta = (TaskFlowCategory = "我的分类")) */
	const FName MetaDataKey_NodeCategory(TEXT("TaskFlowCategory"));
}
