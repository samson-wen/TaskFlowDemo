#include "TaskFlowActorReference.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "TaskFlowTypes.h"

bool FTaskFlowActorReference::IsConfigured() const
{
	return !Actor.IsNull() || !ActorTag.IsNone();
}

bool FTaskFlowActorReference::MatchesClass(const AActor* Candidate) const
{
	if (Candidate == nullptr)
	{
		return false;
	}

	if (ActorClass.IsNull())
	{
		return true;
	}

	UClass* ExpectedClass = ActorClass.LoadSynchronous();
	return ExpectedClass != nullptr && Candidate->IsA(ExpectedClass);
}

AActor* FTaskFlowActorReference::GetActor(const UObject* WorldContextObject) const
{
	// 1. Soft path. TSoftObjectPtr::Get() applies FixupForPIE in PIE, so the level Actor
	// selected in the editor resolves to its PIE counterpart rather than the editor-world instance.
	if (!Actor.IsNull())
	{
		if (AActor* Resolved = Actor.Get())
		{
			if (MatchesClass(Resolved))
			{
				return Resolved;
			}
		}
	}

	UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	if (World == nullptr)
	{
		return nullptr;
	}

	// 2. Stable tag fallback. This is the recommended path for packaged builds / World Partition,
	// where the editor level path may no longer resolve.
	if (!ActorTag.IsNone())
	{
		TArray<AActor*> TaggedActors;
		UGameplayStatics::GetAllActorsWithTag(World, ActorTag, TaggedActors);
		for (AActor* Candidate : TaggedActors)
		{
			if (IsValid(Candidate) && MatchesClass(Candidate))
			{
				return Candidate;
			}
		}
	}

	// 3. Object-name fallback derived from the soft path, e.g.
	// "/Game/Maps/MyMap.MyMap:PersistentLevel.MyActor_C_1" -> "MyActor_C_1".
	// Actor object names survive PIE duplication, so this works even when the package path differs.
	const FString Path = Actor.ToSoftObjectPath().ToString();
	int32 LastDot = INDEX_NONE;
	if (Path.FindLastChar(TEXT('.'), LastDot))
	{
		const FName ActorName(*Path.Mid(LastDot + 1));
		if (!ActorName.IsNone())
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				AActor* Candidate = *It;
				if (IsValid(Candidate) && Candidate->GetFName() == ActorName && MatchesClass(Candidate))
				{
					return Candidate;
				}
			}
		}
	}

	UE_LOG(LogTaskFlow, Verbose, TEXT("[TaskFlow] ActorReference could not resolve (Actor=%s, Tag=%s)"),
		*Path, *ActorTag.ToString());

	return nullptr;
}
