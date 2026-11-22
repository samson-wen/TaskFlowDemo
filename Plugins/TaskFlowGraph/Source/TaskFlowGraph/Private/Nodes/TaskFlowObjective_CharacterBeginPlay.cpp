#include "Nodes/TaskFlowObjective_CharacterBeginPlay.h"

#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "TaskFlowRunner.h"
#include "TaskFlowTypes.h"

#define LOCTEXT_NAMESPACE "TaskFlowObjective_CharacterBeginPlay"

UTaskFlowObjective_CharacterBeginPlay::UTaskFlowObjective_CharacterBeginPlay()
{
	ObjectiveDescription = LOCTEXT("DefaultDescription", "Character Begin Play");

	// Must be >= 1: with 0 the base class would complete the node before any Character begins play.
	MaxProgress = 1;
}

FText UTaskFlowObjective_CharacterBeginPlay::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Watches for a Character's BeginPlay through the engine Pawn delegate, then runs Actions with that Character as the flow context.");
}

void UTaskFlowObjective_CharacterBeginPlay::Execute_Implementation(UTaskFlowRunner* Runner)
{
	// Node instances live on a shared asset: clear any delegate left by a previous execution.
	UnsubscribeFromBeginPlay();
	MaxProgress = FMath::Max(1, MaxProgress);

	// The base class binds no task-flow event here (bListenToTaskFlowEvents is false) and skips
	// Actions because ShouldDeferActionsUntilEvent() is true.
	Super::Execute_Implementation(Runner);

	// APawn::OnPawnBeginPlay is static, so one registration observes every Pawn in every World.
	PawnBeginPlayHandle = APawn::OnPawnBeginPlay.AddUObject(
		this, &UTaskFlowObjective_CharacterBeginPlay::HandlePawnBeginPlay);
}

bool UTaskFlowObjective_CharacterBeginPlay::ShouldDeferActionsUntilEvent() const
{
	return true;
}

void UTaskFlowObjective_CharacterBeginPlay::HandlePawnBeginPlay(APawn* Pawn)
{
	ACharacter* Character = Cast<ACharacter>(Pawn);
	if (!IsValid(Character))
	{
		return;
	}

	if (CharacterClass != nullptr && !Character->IsA(CharacterClass))
	{
		return;
	}

	UTaskFlowRunner* Runner = GetRunner();
	if (Runner == nullptr)
	{
		return;
	}

	// APawn::OnPawnBeginPlay is static and broadcasts for every World; ignore Pawns belonging to
	// another (e.g. PIE client) World, otherwise SetContext would inject a foreign-World Actor.
	if (UWorld* FlowWorld = Runner->GetTaskFlowWorld())
	{
		if (Pawn->GetWorld() != FlowWorld)
		{
			return;
		}
	}

	UnsubscribeFromBeginPlay();

	// Actions read this through GetContext(); the original task context is intentionally replaced
	// by the Character that began play, which is what this Objective is for.
	Runner->SetContext(Character);

	StartActions();

	if (MaxProgress > 0)
	{
		AddProgress(1);
	}

	const FString Detail = FString::Printf(TEXT("Character=%s Progress=%d/%d"),
		*GetNameSafe(Character), CurrentProgress, MaxProgress);
	LogTaskFlowLifecycle(
		TEXT("<None>"), TEXT("BeginPlayAccepted"), GetName(),
		*GetClass()->GetName(), *Detail);
}

void UTaskFlowObjective_CharacterBeginPlay::OnFlowEnded()
{
	// Flow stopped / reached End before any Character began play: release the delegate.
	UnsubscribeFromBeginPlay();
	Super::OnFlowEnded();
}

void UTaskFlowObjective_CharacterBeginPlay::UnsubscribeFromBeginPlay()
{
	if (PawnBeginPlayHandle.IsValid())
	{
		APawn::OnPawnBeginPlay.Remove(PawnBeginPlayHandle);
		PawnBeginPlayHandle.Reset();
	}
}

#undef LOCTEXT_NAMESPACE
