#include "AssetDefinition_TaskFlow.h"

#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "TaskFlowAsset.h"
#include "TaskFlowGraphEditorModule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_TaskFlow)

#define LOCTEXT_NAMESPACE "AssetDefinition_TaskFlow"

FText UAssetDefinition_TaskFlow::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDefinition_TaskFlow_DisplayName", "Task Flow");
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
		FAssetCategoryPath(EAssetCategoryPaths::Gameplay, LOCTEXT("TaskFlow_SubMenu", "Task Flow"), ECategoryMenuType::Section)
	};

	return Categories;
}

EAssetCommandResult UAssetDefinition_TaskFlow::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	FTaskFlowGraphEditorModule& EditorModule = FModuleManager::GetModuleChecked<FTaskFlowGraphEditorModule>("TaskFlowGraphEditor");

	for (UTaskFlowAsset* Asset : OpenArgs.LoadObjects<UTaskFlowAsset>())
	{
		// If the asset is already open, focus the existing editor instead of opening a duplicate.
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
