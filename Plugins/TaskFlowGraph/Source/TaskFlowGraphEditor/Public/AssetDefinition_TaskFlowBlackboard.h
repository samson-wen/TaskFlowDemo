#pragma once

#include "AssetDefinitionDefault.h"
#include "AssetDefinition_TaskFlowBlackboard.generated.h"

/** Lets the Content Browser recognize UTaskFlowBlackboard; double-click opens it in the default property editor to edit entries. */
UCLASS()
class UAssetDefinition_TaskFlowBlackboard : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	//~ Begin UAssetDefinition Interface
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	//~ End UAssetDefinition Interface
};
