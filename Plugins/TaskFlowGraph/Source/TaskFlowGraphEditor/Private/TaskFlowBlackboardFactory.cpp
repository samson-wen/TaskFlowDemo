#include "TaskFlowBlackboardFactory.h"

#include "TaskFlowBlackboard.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TaskFlowBlackboardFactory)

#define LOCTEXT_NAMESPACE "TaskFlowBlackboardFactory"

UTaskFlowBlackboardFactory::UTaskFlowBlackboardFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UTaskFlowBlackboard::StaticClass();
}

UObject* UTaskFlowBlackboardFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UTaskFlowBlackboard>(InParent, InClass, InName, Flags | RF_Transactional);
}

FText UTaskFlowBlackboardFactory::GetDisplayName() const
{
	return LOCTEXT("TaskFlowBlackboardFactory_DisplayName", "任务黑板");
}

bool UTaskFlowBlackboardFactory::ShouldShowInNewMenu() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
