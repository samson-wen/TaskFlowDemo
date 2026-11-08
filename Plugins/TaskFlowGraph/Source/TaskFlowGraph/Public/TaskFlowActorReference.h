#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "TaskFlowActorReference.generated.h"

class AActor;
class UObject;

/**
 * Configurable reference to an Actor placed in a level.
 *
 * In the Details panel, the Actor field provides UE's native scene picker (eye-dropper and
 * Scene Outliner list), so a level Actor can be selected directly from a TaskFlow asset.
 * It is stored as a soft reference to avoid a hard package dependency from the asset to a map.
 *
 * At runtime, GetActor() resolves it in the world of the supplied context:
 *   1. the soft path (works in editor / PIE via FixupForPIE, and in packaged builds when the map is loaded);
 *   2. ActorTag, when configured (stable across packaged builds, World Partition and other levels);
 *   3. the Actor object name extracted from the soft path (packaged-build fallback).
 *
 * Do not store a hard AActor* in an asset: it would keep pointing at the editor-world Actor in PIE.
 */
USTRUCT(BlueprintType)
struct TASKFLOWGRAPH_API FTaskFlowActorReference
{
	GENERATED_BODY()

	/** Select an Actor from the current level. Soft reference, safe to store in a TaskFlow asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow|Actor", meta = (AllowedClasses = "/Script/Engine.Actor", Untracked))
	TSoftObjectPtr<AActor> Actor;

	/**
	 * Optional stable fallback. Add this ActorTag to the target Actor in the level and set it here;
	 * resolution then works even when the soft level path cannot be resolved.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow|Actor")
	FName ActorTag = NAME_None;

	/** Optional class filter applied to every resolution path. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskFlow|Actor")
	TSoftClassPtr<AActor> ActorClass;

	/** True when at least one way of locating an Actor has been configured. */
	bool IsConfigured() const;

	/**
	 * Resolve the configured Actor in the world of WorldContextObject.
	 * Pass a UWorld, an in-world Actor/Component, or the TaskFlow Runner's world context.
	 * Returns nullptr when the world is unavailable or no matching Actor exists.
	 */
	AActor* GetActor(const UObject* WorldContextObject) const;

private:
	/** Whether the candidate satisfies the optional ActorClass filter. */
	bool MatchesClass(const AActor* Candidate) const;
};
