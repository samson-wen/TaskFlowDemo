// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/Volume.h"
#include "TaskFlowVolume.generated.h"


// Blueprintable: can be chosen as a parent class when creating a Blueprint.
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

	//      Developers can override OnCharacterOverlapBegin_Implementation in C++ or override the method in Blueprint.
	// Called when a Character begins overlapping this volume (only if bEnableCharacterOverlapEvent).
	//      Implement details in C++ (_Implementation) or in Blueprint.
	UFUNCTION(BlueprintNativeEvent, Category = "Volume Event")
	void OnCharacterOverlapBegin(ACharacter* Character);

	// Called when a character leaves the volume (only when bEnableCharacterOverlapEvent is true).
	UFUNCTION(BlueprintNativeEvent, Category = "Volume Event")
	void OnCharacterOverlapEnd(ACharacter* Character);

private:
	UFUNCTION()
	void HandleActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void HandleActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
