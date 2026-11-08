#include "AssetDefinition_TaskFlowBlackboard.h"

#include "TaskFlowBlackboard.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_TaskFlowBlackboard)

#define LOCTEXT_NAMESPACE "AssetDefinition_TaskFlowBlackboard"

FText UAssetDefinition_TaskFlowBlackboard::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDefinition_TaskFlowBlackboard_DisplayName", "Task Flow Blackboard");
}

FLinearColor UAssetDefinition_TaskFlowBlackboard::GetAssetColor() const
{
	return FLinearColor(FColor(96, 96, 128));
}

TSoftClassPtr<UObject> UAssetDefinition_TaskFlowBlackboard::GetAssetClass() const
{
	return UTaskFlowBlackboard::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_TaskFlowBlackboard::GetAssetCategories() const
{
	static const FAssetCategoryPath Categories[] =
	{
		FAssetCategoryPath(EAssetCategoryPaths::Gameplay, LOCTEXT("TaskFlow_SubMenu", "Task Flow"), ECategoryMenuType::Section)
	};

	return Categories;
}

#undef LOCTEXT_NAMESPACE
