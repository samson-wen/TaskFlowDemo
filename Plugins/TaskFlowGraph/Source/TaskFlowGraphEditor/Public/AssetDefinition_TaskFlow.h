#pragma once

#include "AssetDefinitionDefault.h"
#include "AssetDefinition_TaskFlow.generated.h"

/** Lets the Content Browser recognize UTaskFlowAsset: display name, color, category, and opening the graph editor on double-click. */
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
