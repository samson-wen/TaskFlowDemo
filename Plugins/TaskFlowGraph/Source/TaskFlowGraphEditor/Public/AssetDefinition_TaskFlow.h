#pragma once

#include "AssetDefinitionDefault.h"
#include "AssetDefinition_TaskFlow.generated.h"

/** 让内容浏览器认识 UTaskFlowAsset：显示名、颜色、分类，以及双击打开图编辑器 */
UCLASS()
class UAssetDefinition_TaskFlow : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	//~ Begin UAssetDefinition Interface
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual EAssetCommandResult OpenAssets(const FAssetOpenArgs& OpenArgs) const override;
	virtual EAssetCommandResult ActivateAssets(const FAssetActivateArgs& ActivateArgs) const override;
	//~ End UAssetDefinition Interface
};
