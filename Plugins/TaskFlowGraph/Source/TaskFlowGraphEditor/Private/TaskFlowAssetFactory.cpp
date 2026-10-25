#include "TaskFlowAssetFactory.h"

#include "TaskFlowAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TaskFlowAssetFactory)

#define LOCTEXT_NAMESPACE "TaskFlowAssetFactory"

UTaskFlowAssetFactory::UTaskFlowAssetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UTaskFlowAsset::StaticClass();
}

UObject* UTaskFlowAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UTaskFlowAsset* NewAsset = NewObject<UTaskFlowAsset>(InParent, InClass, InName, Flags | RF_Transactional);
	NewAsset->CreateOrLoadGraph();

	return NewAsset;
}

FText UTaskFlowAssetFactory::GetDisplayName() const
{
	return LOCTEXT("TaskFlowAssetFactory_DisplayName", "任务流程");
}

bool UTaskFlowAssetFactory::ShouldShowInNewMenu() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
