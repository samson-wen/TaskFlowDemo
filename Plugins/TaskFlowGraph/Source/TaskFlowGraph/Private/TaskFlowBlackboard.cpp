#include "TaskFlowBlackboard.h"

// Implementation of UTaskFlowBlackboard. The only custom logic is FindEntry, which searches Entries by KeyName.
const FTaskFlowBlackboardEntry* UTaskFlowBlackboard::FindEntry(FName Key) const
{
	return Entries.FindByPredicate([Key](const FTaskFlowBlackboardEntry& Entry)
	{
		return Entry.KeyName == Key;
	});
}
