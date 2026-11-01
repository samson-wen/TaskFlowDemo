#include "TaskFlowBlackboard.h"

// [EN] Implementation of UTaskFlowBlackboard. The only custom logic is FindEntry, which searches Entries by KeyName.
// [CN] UTaskFlowBlackboard 的实现。唯一的自定义逻辑是 FindEntry，按 KeyName 在 Entries 中查找。
const FTaskFlowBlackboardEntry* UTaskFlowBlackboard::FindEntry(FName Key) const
{
	return Entries.FindByPredicate([Key](const FTaskFlowBlackboardEntry& Entry)
	{
		return Entry.KeyName == Key;
	});
}
