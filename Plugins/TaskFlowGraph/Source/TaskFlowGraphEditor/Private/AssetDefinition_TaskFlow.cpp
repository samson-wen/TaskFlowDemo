#include "AssetDefinition_TaskFlow.h"

#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "TaskFlowAsset.h"
#include "TaskFlowGraphEditorModule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_TaskFlow)

#define LOCTEXT_NAMESPACE "AssetDefinition_TaskFlow"

FText UAssetDefinition_TaskFlow::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDefinition_TaskFlow_DisplayName", "任务流程");
}

FLinearColor UAssetDefinition_TaskFlow::GetAssetColor() const
{
	return FLinearColor(FColor(201, 144, 62));
}

TSoftClassPtr<UObject> UAssetDefinition_TaskFlow::GetAssetClass() const
{
	return UTaskFlowAsset::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_TaskFlow::GetAssetCategories() const
{
	static const FAssetCategoryPath Categories[] =
	{
		FAssetCategoryPath(EAssetCategoryPaths::Gameplay, LOCTEXT("TaskFlow_SubMenu", "任务流程"), ECategoryMenuType::Section)
	};

	return Categories;
}

EAssetCommandResult UAssetDefinition_TaskFlow::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	FTaskFlowGraphEditorModule& EditorModule = FModuleManager::GetModuleChecked<FTaskFlowGraphEditorModule>("TaskFlowGraphEditor");

	for (UTaskFlowAsset* Asset : OpenArgs.LoadObjects<UTaskFlowAsset>())
	{
		// 若该资产已打开，则聚焦已有编辑器，不重复打开
		if (AssetEditorSubsystem->FindEditorForAsset(Asset, true) != nullptr)
		{
			continue;
		}

		EditorModule.CreateTaskFlowGraphEditor(OpenArgs.GetToolkitMode(), OpenArgs.ToolkitHost, Asset);
	}

	return EAssetCommandResult::Handled;
}

EAssetCommandResult UAssetDefinition_TaskFlow::ActivateAssets(const FAssetActivateArgs& ActivateArgs) const
{
	FAssetOpenArgs OpenArgs;
	OpenArgs.Assets = ActivateArgs.Assets;
	OpenArgs.OpenMethod = EAssetOpenMethod::Edit;

	return OpenAssets(OpenArgs);
}

#undef LOCTEXT_NAMESPACE
