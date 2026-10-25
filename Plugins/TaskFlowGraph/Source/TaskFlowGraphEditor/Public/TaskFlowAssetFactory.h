#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "TaskFlowAssetFactory.generated.h"

/** 在内容浏览器里创建任务流程资产 */
UCLASS(hidecategories = Object, collapsecategories)
class UTaskFlowAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UTaskFlowAssetFactory();

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override;
	virtual bool ShouldShowInNewMenu() const override;
	//~ End UFactory Interface
};
