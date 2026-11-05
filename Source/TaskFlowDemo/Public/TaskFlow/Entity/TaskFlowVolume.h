// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/Volume.h"
#include "TaskFlowVolume.generated.h"


// [CN] Blueprintable: 可作为蓝图父类（在创建蓝图时可选为本类）。
// [EN] Blueprintable: can be chosen as a parent class when creating a Blueprint.
UCLASS(Blueprintable)
class ATaskFlowVolume : public AVolume
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Info")
	int32 VolumeID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Info")
	FString VolumeTag = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Info")
	bool bEnableCharacterOverlapEvent = false;

protected:
	virtual void BeginPlay() override;

	// [CN] 当角色进入体积时调用（仅在 bEnableCharacterOverlapEvent 为 true 时）。
	//      开发者可在 C++ 中覆写 OnCharacterOverlapBegin_Implementation，或在蓝图里覆写该方法以实现细节。
	// [EN] Called when a Character begins overlapping this volume (only if bEnableCharacterOverlapEvent).
	//      Implement details in C++ (_Implementation) or in Blueprint.
	UFUNCTION(BlueprintNativeEvent, Category = "Volume Event")
	void OnCharacterOverlapBegin(ACharacter* Character);

	// [CN] 当角色离开体积时调用（仅在 bEnableCharacterOverlapEvent 为 true 时）。
	UFUNCTION(BlueprintNativeEvent, Category = "Volume Event")
	void OnCharacterOverlapEnd(ACharacter* Character);

private:
	UFUNCTION()
	void HandleActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void HandleActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
