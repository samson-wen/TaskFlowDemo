// Copyright Epic Games, Inc. All Rights Reserved.

#include "TaskFlowVolume.h"

#include "Components/BrushComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"

#include "TaskFlowEvent.h"
#include "TaskFlow/GameTaskFlowTypes.h"
#include "TaskFlowSystem.h"

void ATaskFlowVolume::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableCharacterOverlapEvent)
	{
		// [EN] Ensure the volume's brush reports overlaps so the events actually fire.
		if (UBrushComponent* BrushComp = GetBrushComponent())
		{
			BrushComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			BrushComp->SetGenerateOverlapEvents(true);
		}

		OnActorBeginOverlap.AddDynamic(this, &ATaskFlowVolume::HandleActorBeginOverlap);
		OnActorEndOverlap.AddDynamic(this, &ATaskFlowVolume::HandleActorEndOverlap);
	}
}

void ATaskFlowVolume::OnCharacterOverlapBegin_Implementation(ACharacter* Character)
{
	// [CN] 默认实现：向 TaskFlowSystem 全局广播“角色进入体积”事件。
	//      事件名规则：已设 VolumeTag 时为 "<VolumeTag>_Enter"，否则为 "TaskFlowVolume_<VolumeID>_Enter"。
	//      开发者在蓝图 / C++ 覆写时，可调用 Parent 以保留此广播，或在其中自行接入 TaskFlow。
	if (UTaskFlowSystem* TaskFlowSystem = GetWorld()->GetSubsystem<UTaskFlowSystem>())
	{
		FTaskFlowEvent TaskFlowEvt;
		TaskFlowEvt.EventName = GameTaskFlowEvents::EVENT_TASKFLOW_VOLUME_ENTER;
		TaskFlowEvt.Sender = Character;

		// [CN] 体积专属数据放进类型擦除的 EventData，经动态委托派发时会被深拷贝，不会被切片。
		FTaskFlowVolumeEvent& VolumeEvt = TaskFlowEvt.EventData.InitializeAs<FTaskFlowVolumeEvent>();
		VolumeEvt.VolumeID = this->VolumeID;
		VolumeEvt.VolumeTag = this->VolumeTag;

		TaskFlowSystem->BroadcastTaskFlowEvent(TaskFlowEvt);
	}
}

void ATaskFlowVolume::OnCharacterOverlapEnd_Implementation(ACharacter* Character)
{
	// [CN] 默认实现：向 TaskFlowSystem 全局广播“角色离开体积”事件。
	if (UTaskFlowSystem* TaskFlowSystem = GetWorld()->GetSubsystem<UTaskFlowSystem>())
	{
		FTaskFlowEvent TaskFlowEvt;
		TaskFlowEvt.EventName = GameTaskFlowEvents::EVENT_TASKFLOW_VOLUME_LEAVE;
		TaskFlowEvt.Sender = Character;

		FTaskFlowVolumeEvent& VolumeEvt = TaskFlowEvt.EventData.InitializeAs<FTaskFlowVolumeEvent>();
		VolumeEvt.VolumeID = this->VolumeID;
		VolumeEvt.VolumeTag = this->VolumeTag;

		TaskFlowSystem->BroadcastTaskFlowEvent(TaskFlowEvt);
	}
}

void ATaskFlowVolume::HandleActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		OnCharacterOverlapBegin(Character);
	}
}

void ATaskFlowVolume::HandleActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (ACharacter* Character = Cast<ACharacter>(OtherActor))
	{
		OnCharacterOverlapEnd(Character);
	}
}
