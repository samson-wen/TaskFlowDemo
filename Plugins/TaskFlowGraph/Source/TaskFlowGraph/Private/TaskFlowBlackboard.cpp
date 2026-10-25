#include "TaskFlowBlackboard.h"

const FTaskFlowBlackboardEntry* UTaskFlowBlackboard::FindEntry(FName Key) const
{
	return Entries.FindByPredicate([Key](const FTaskFlowBlackboardEntry& Entry)
	{
		return Entry.KeyName == Key;
	});
}
