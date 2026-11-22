// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "PlayerStateTaskComponent.generated.h"


UCLASS(Blueprintable)
class UPlayerStateTaskComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerStateTaskComponent();

	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void AcceptTask(int32 TaskId);

public:
	UPROPERTY(BlueprintReadOnly)
	TArray<int32> AcceptedTaskIds;
	UPROPERTY(BlueprintReadWrite)
	TArray<int32> CompletedTaskIds;
	/** Task table whose rows use FGameTask as the row struct; looked up by TaskID on accept. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDataTable> TaskDataTable;
};
