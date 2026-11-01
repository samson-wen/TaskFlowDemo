#pragma once

#include "AssetDefinitionDefault.h"
#include "AssetDefinition_TaskFlowBlackboard.generated.h"

/** [EN] Lets the Content Browser recognize UTaskFlowBlackboard; double-click opens it in the default property editor to edit entries. [CN] 让内容浏览器认识 UTaskFlowBlackboard，双击用默认属性编辑器编辑条目 */
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
