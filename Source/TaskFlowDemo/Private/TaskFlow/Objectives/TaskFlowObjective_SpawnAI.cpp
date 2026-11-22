#include "TaskFlow/Objectives/TaskFlowObjective_SpawnAI.h"

#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "TaskFlow/GameTaskFlowTypes.h"
#include "TaskFlowRunner.h"
#include "TaskFlowTypes.h"
#include "Variant_Combat/AI/CombatEnemy.h"

#define LOCTEXT_NAMESPACE "TaskFlowObjective_SpawnAI"

namespace
{
	/**
	 * Resolve one ground-level spawn point. A single AI uses the center directly; when spawning
	 * several, prefer a random reachable point on the NavMesh and fall back to a uniform random
	 * offset inside the same radius when navigation data is unavailable.
	 */
	FVector ResolveSpawnLocation(UWorld* World, const FVector& Center, float Range, bool bScatter)
	{
		if (!bScatter)
		{
			return Center;
		}

		FVector NavPoint = FVector::ZeroVector;
		if (UNavigationSystemV1::K2_GetRandomReachablePointInRadius(World, Center, NavPoint, Range))
		{
			return NavPoint;
		}

		UE_LOG(LogTaskFlow, Verbose,
			TEXT("[TaskFlow] SpawnAI: no reachable NavMesh point around %s (radius %.0f cm); using a random offset."),
			*Center.ToString(), Range);

		const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
		const float Distance = FMath::Sqrt(FMath::FRand()) * FMath::Max(0.0f, Range);
		return Center + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
	}
}

UTaskFlowObjective_SpawnAI::UTaskFlowObjective_SpawnAI()
{
	ObjectiveDescription = LOCTEXT("DefaultDescription", "Spawn AI");

	// Seed the inherited progress target from the configured spawn count. MaxProgress = 0 would let
	// the base class complete immediately when this Objective has no Actions.
	MaxProgress = SpawnNum;

	// Forward events to this node's Actions while executing; the base class binds / unbinds the delegate.
	bListenToTaskFlowEvents = true;
}

FText UTaskFlowObjective_SpawnAI::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Spawns the configured number of AI across frames (SpawnPerFrame per tick) around the spawn center, using random navigable points within SpawnRange when spawning multiple, and completes once they are created.");
}

void UTaskFlowObjective_SpawnAI::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// SpawnNum is the authoritative spawn count; sync it into the inherited progress target so
	// Details-panel edits take effect at execution time (the constructor only seeds it).
	MaxProgress = FMath::Max(1, SpawnNum);

	// This node lives on a shared asset, so clear any state left by a previous execution first.
	ClearSpawnTimer();
	bSpawnCancelled = false;
	SpawnedCount = 0;
	PendingSpawnCount = MaxProgress;

	// The base Execute resets progress, forwards events and starts the Actions. With MaxProgress >= 1
	// it keeps this Objective executing until FinishSpawning() drives the progress update.
	Super::Execute_Implementation(Runner);

	UWorld* World = Runner != nullptr ? Runner->GetTaskFlowWorld() : nullptr;
	if (World == nullptr || !IsValid(EnemyClass))
	{
		UE_LOG(LogTaskFlow, Warning,
			TEXT("[TaskFlow] SpawnAI '%s': invalid World or EnemyClass, no AI spawned."), *GetName());
		FinishSpawning();
		return;
	}

	SpawnWorld = World;

	// SpawnPointRef wins over the manually entered position; both describe a ground-level center.
	CachedSpawnCenter = SpawnPosition;
	CachedSpawnRotation = FRotator::ZeroRotator;
	if (AActor* SpawnPoint = SpawnPointRef.GetActor(World))
	{
		CachedSpawnCenter = SpawnPoint->GetActorLocation();
		CachedSpawnRotation = SpawnPoint->GetActorRotation();
	}

	// ACharacter's location is the capsule center, while NavMesh points lie on the ground.
	CachedCapsuleHalfHeight = 88.0f;
	if (const ACharacter* EnemyCDO = Cast<ACharacter>(EnemyClass->GetDefaultObject()))
	{
		if (const UCapsuleComponent* Capsule = EnemyCDO->GetCapsuleComponent())
		{
			CachedCapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}
	}

	// Spread the SpawnActor calls over frames instead of doing them all inside one Execute call.
	ScheduleNextSpawn();
}

void UTaskFlowObjective_SpawnAI::OnFlowEnded()
{
	// The flow was stopped / reached End before every AI was spawned: do not create more.
	bSpawnCancelled = true;
	ClearSpawnTimer();
	Super::OnFlowEnded();
}

void UTaskFlowObjective_SpawnAI::HandleSpawnTick()
{
	SpawnTimerHandle.Invalidate();

	if (bSpawnCancelled)
	{
		return;
	}

	UWorld* World = SpawnWorld.Get();
	if (World == nullptr || !IsValid(EnemyClass))
	{
		UE_LOG(LogTaskFlow, Warning,
			TEXT("[TaskFlow] SpawnAI '%s': spawn aborted because the World or EnemyClass became invalid."), *GetName());
		FinishSpawning();
		return;
	}

	const bool bScatter = MaxProgress > 1;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const int32 BatchSize = FMath::Max(1, SpawnPerFrame);
	for (int32 Batch = 0; Batch < BatchSize && PendingSpawnCount > 0; ++Batch)
	{
		--PendingSpawnCount;

		const FVector GroundPoint = ResolveSpawnLocation(World, CachedSpawnCenter, SpawnRange, bScatter);
		const FVector SpawnLocation = GroundPoint + FVector(0.0f, 0.0f, CachedCapsuleHalfHeight);

		ACombatEnemy* SpawnedEnemy = World->SpawnActor<ACombatEnemy>(
			EnemyClass, SpawnLocation, CachedSpawnRotation, SpawnParams);

		if (SpawnedEnemy == nullptr)
		{
			UE_LOG(LogTaskFlow, Warning,
				TEXT("[TaskFlow] SpawnAI '%s': SpawnActor failed at %s."), *GetName(), *SpawnLocation.ToString());
			continue;
		}

		SpawnedEnemy->AIID = AIID;
		if (!AITag.IsEmpty())
		{
			SpawnedEnemy->Tags.AddUnique(FName(*AITag));
		}
		++SpawnedCount;
	}

	if (PendingSpawnCount > 0)
	{
		ScheduleNextSpawn();
	}
	else
	{
		FinishSpawning();
	}
}

void UTaskFlowObjective_SpawnAI::ScheduleNextSpawn()
{
	UWorld* World = SpawnWorld.Get();
	if (World == nullptr || bSpawnCancelled)
	{
		FinishSpawning();
		return;
	}

	SpawnTimerHandle = World->GetTimerManager().SetTimerForNextTick(
		this, &UTaskFlowObjective_SpawnAI::HandleSpawnTick);

	// Never leave the Objective stuck if the timer could not be queued.
	if (!SpawnTimerHandle.IsValid())
	{
		UE_LOG(LogTaskFlow, Warning,
			TEXT("[TaskFlow] SpawnAI '%s': failed to schedule the next spawn tick."), *GetName());
		FinishSpawning();
	}
}

void UTaskFlowObjective_SpawnAI::ClearSpawnTimer()
{
	if (UWorld* World = SpawnWorld.Get())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
	SpawnTimerHandle.Invalidate();
}

void UTaskFlowObjective_SpawnAI::FinishSpawning()
{
	// Clear state BEFORE SetProgress: completing can synchronously advance the flow and even
	// re-enter Execute on this shared asset node.
	ClearSpawnTimer();
	SpawnWorld.Reset();
	PendingSpawnCount = 0;

	UE_LOG(LogTaskFlow, Log,
		TEXT("[TaskFlow] SpawnAI '%s': spawned %d/%d (AIID=%lld, Center=%s, Range=%.0f cm, PerFrame=%d)"),
		*GetName(), SpawnedCount, MaxProgress, AIID, *CachedSpawnCenter.ToString(), SpawnRange, SpawnPerFrame);

	// One progress update completes the Objective exactly once; asynchronous Actions are still
	// awaited by the base class, and a failed spawn can never leave the flow stuck.
	SetProgress(MaxProgress);
}

#undef LOCTEXT_NAMESPACE
