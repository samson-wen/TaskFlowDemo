// Copyright Epic Games, Inc. All Rights Reserved.

#include "TaskFlowVolume.h"

#include "Components/BrushComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"

#include "TaskFlowEvent.h"
#include "TaskFlowSystem.h"
#include "TaskFlow/GameTaskFlowTypes.h"

void ATaskFlowVolume::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableCharacterOverlapEvent && this->HasAuthority())
	{
		// Ensure the volume's brush reports overlaps so the events actually fire.
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
	// Default implementation: broadcast a 'character entered volume' event through TaskFlowSystem.
	//      Event name rule: "<VolumeTag>_Enter" when VolumeTag is set, otherwise "TaskFlowVolume_<VolumeID>_Enter".
	//      When overriding in Blueprint / C++, call the parent to keep this broadcast, or wire up TaskFlow yourself.
	if (UTaskFlowSystem* TaskFlowSystem = GetWorld()->GetSubsystem<UTaskFlowSystem>())
	{
		FTaskFlowEvent TaskFlowEvt;
		TaskFlowEvt.EventName = GameTaskFlowEvents::EVENT_TASKFLOW_VOLUME_ENTER;
		TaskFlowEvt.Sender = Character;

		// Put volume-specific data into the type-erased EventData; the dynamic delegate deep-copies it, so it is not sliced.
		FTaskFlowVolumeEvent& VolumeEvt = TaskFlowEvt.EventData.InitializeAs<FTaskFlowVolumeEvent>();
		VolumeEvt.VolumeID = this->VolumeID;
		VolumeEvt.VolumeTag = this->VolumeTag;

		TaskFlowSystem->BroadcastTaskFlowEvent(TaskFlowEvt);
	}
}

void ATaskFlowVolume::OnCharacterOverlapEnd_Implementation(ACharacter* Character)
{
	// Default implementation: broadcast a 'character left volume' event through TaskFlowSystem.
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

void ATaskFlowVolume::ShowArrow()
{
	UArrowComponent* ArrowComp = this->GetComponentByClass<UArrowComponent>();
	if (ArrowComp != nullptr)
	{
		ArrowComp->SetHiddenInGame(false);
	}
}

void ATaskFlowVolume::HideArrow()
{
	UArrowComponent* ArrowComp = this->GetComponentByClass<UArrowComponent>();
	if (ArrowComp != nullptr)
	{
		ArrowComp->SetHiddenInGame(true);
	}

}
