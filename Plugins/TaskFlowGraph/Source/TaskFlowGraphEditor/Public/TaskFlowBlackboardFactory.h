#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "TaskFlowBlackboardFactory.generated.h"

/** Creates a Task Flow Blackboard asset in the Content Browser. */
UCLASS(hidecategories = Object, collapsecategories)
class UTaskFlowBlackboardFactory : public UFactory
{
	GENERATED_BODY()

public:
	UTaskFlowBlackboardFactory();

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override;
	virtual bool ShouldShowInNewMenu() const override;
	//~ End UFactory Interface
};
